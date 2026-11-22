/***************************************************************************

    retrothread.c - OSD locks and work queues on libretro-common

    osd_lock is a recursive lock: an atomic state word, with contended
    acquirers parked on a retro_eventcount.  Work queues run their items
    on a tpool; finished items go back to the queue through a retro_mpsc
    free list, and each running callback holds one of WORK_MAX_THREADS
    threadid slots, claimed from an atomic bitmask.

    Without HAVE_THREADS or pointer atomics, locks do nothing and work
    items run on the thread that queues them.

***************************************************************************/

#include <stdlib.h>

#ifndef INLINE
#ifdef _MSC_VER
#define INLINE static __inline
#else
#define INLINE static __inline__
#endif
#endif
#include "osdcore.h"
#include "retroos.h"
/* osdcore.h is done with the core's INLINE; libretro-common has its own */
#undef INLINE

#include <retro_atomic.h>

#if defined(HAVE_THREADS) && defined(RETRO_ATOMIC_HAS_PTR)
#define RETRO_OSD_THREADED 1
#include <retro_mpsc.h>
#include <rthreads/rthreads.h>
#include <rthreads/retro_eventcount.h>
#include <rthreads/tpool.h>
#endif

/* 0 for one thread per host processor, otherwise the processor count to
 * assume; set from the core options before machine init */
int mosd_num_processors = 0;

#ifdef RETRO_OSD_THREADED

/*-------------------------------------------------
    locks
-------------------------------------------------*/

struct _osd_lock
{
   retro_eventcount_t ec;     /* contended acquirers park here          */
   retro_atomic_ptr_t owner;  /* thread id of the holder, or NULL       */
   retro_atomic_int_t state;  /* 0 free, 1 held, 2 held with waiters    */
   int                depth;  /* recursion count, touched by the holder */
};

osd_lock *osd_lock_alloc(void)
{
   osd_lock *lock = (osd_lock *)calloc(1, sizeof(*lock));
   if (!lock)
      return NULL;
   if (!retro_eventcount_init(&lock->ec))
   {
      free(lock);
      return NULL;
   }
   retro_atomic_ptr_init(&lock->owner, NULL);
   retro_atomic_int_init(&lock->state, 0);
   return lock;
}

static void osd_lock_contended(osd_lock *lock)
{
   /* Mark the lock contended whenever trying for it, so the holder's
    * release always notifies while anyone may be parked. */
   while (retro_atomic_exchange_int(&lock->state, 2) != 0)
   {
      int key = retro_eventcount_prepare_wait(&lock->ec);
      if (retro_atomic_load_acquire_int(&lock->state) != 2)
         retro_eventcount_cancel_wait(&lock->ec);
      else
         retro_eventcount_commit_wait(&lock->ec, key);
   }
}

void osd_lock_acquire(osd_lock *lock)
{
   void *self = (void *)sthread_get_current_thread_id();

   /* only this thread ever stores its own id here */
   if (retro_atomic_load_relaxed_ptr(&lock->owner) == self)
   {
      lock->depth++;
      return;
   }
   if (!retro_atomic_cas_int(&lock->state, 0, 1))
      osd_lock_contended(lock);
   retro_atomic_store_relaxed_ptr(&lock->owner, self);
   lock->depth = 1;
}

int osd_lock_try(osd_lock *lock)
{
   void *self = (void *)sthread_get_current_thread_id();

   if (retro_atomic_load_relaxed_ptr(&lock->owner) == self)
   {
      lock->depth++;
      return TRUE;
   }
   if (!retro_atomic_cas_int(&lock->state, 0, 1))
      return FALSE;
   retro_atomic_store_relaxed_ptr(&lock->owner, self);
   lock->depth = 1;
   return TRUE;
}

void osd_lock_release(osd_lock *lock)
{
   if (--lock->depth)
      return;
   retro_atomic_store_relaxed_ptr(&lock->owner, NULL);
   if (retro_atomic_exchange_int(&lock->state, 0) == 2)
      retro_eventcount_notify(&lock->ec);
}

void osd_lock_free(osd_lock *lock)
{
   if (!lock)
      return;
   retro_eventcount_free(&lock->ec);
   free(lock);
}

/*-------------------------------------------------
    work queues
-------------------------------------------------*/

struct _osd_work_item
{
   retro_mpsc_node_t   node;     /* free list link; first, for the cast   */
   osd_work_queue     *queue;
   osd_work_callback   callback;
   void               *param;
   void               *result;
   uint32_t            flags;
   retro_atomic_int_t  done;
};

struct _osd_work_queue
{
   tpool_t            *pool;     /* NULL: items run on the queueing thread */
   osd_work_item      *spare;    /* free items, the queueing thread's own  */
   retro_mpsc_t        released; /* items released from any thread         */
   retro_eventcount_t  done_ec;  /* notified as items complete            */
   retro_atomic_int_t  items;    /* queued or running                     */
   retro_atomic_int_t  slots;    /* threadids held by running callbacks   */
   uint32_t            flags;
};

/* the slot mask is an int; one bit per threadid */
typedef char osd_work_slots_fit[(WORK_MAX_THREADS <= 31) ? 1 : -1];

static int osd_work_slot_claim(osd_work_queue *queue)
{
   for (;;)
   {
      int mask = retro_atomic_load_relaxed_int(&queue->slots);
      int slot;
      for (slot = 0; slot < WORK_MAX_THREADS; slot++)
         if (!(mask & (1 << slot)))
            break;
      /* no more callbacks run at once than there are slots */
      if (slot == WORK_MAX_THREADS)
         sthread_yield();
      else if (retro_atomic_cas_int(&queue->slots, mask, mask | (1 << slot)))
         return slot;
   }
}

static void osd_work_item_run(void *arg)
{
   osd_work_item  *item  = (osd_work_item *)arg;
   osd_work_queue *queue = item->queue;
   int             slot  = osd_work_slot_claim(queue);
   void           *result = (*item->callback)(item->param, slot);

   retro_atomic_fetch_and_int(&queue->slots, ~(1 << slot));

   /* the item may be reused as soon as it is released or marked done */
   if (item->flags & WORK_ITEM_FLAG_AUTO_RELEASE)
   {
      retro_mpsc_push(&queue->released, &item->node);
      if (retro_atomic_fetch_sub_int(&queue->items, 1) == 1)
         retro_eventcount_notify(&queue->done_ec);
   }
   else
   {
      item->result = result;
      retro_atomic_store_release_int(&item->done, 1);
      retro_atomic_fetch_sub_int(&queue->items, 1);
      retro_eventcount_notify(&queue->done_ec);
   }
}

/* Spare items belong to the thread queueing work; released ones are
 * taken over in one exchange when the spares run out. */
static osd_work_item *osd_work_item_get(osd_work_queue *queue)
{
   osd_work_item *item = queue->spare;
   if (!item)
      item = (osd_work_item *)retro_mpsc_take_all(&queue->released);
   if (item)
   {
      queue->spare = (osd_work_item *)item->node.next;
      return item;
   }
   return (osd_work_item *)malloc(sizeof(*item));
}

static void osd_work_item_list_free(osd_work_item *item)
{
   while (item)
   {
      osd_work_item *next = (osd_work_item *)item->node.next;
      free(item);
      item = next;
   }
}

/* waits for the queue to be empty, or for one item, without a bound:
 * items always finish */
static void osd_work_wait_done(osd_work_queue *queue, osd_work_item *item)
{
   for (;;)
   {
      int key;
      if (item ? retro_atomic_load_acquire_int(&item->done)
               : !retro_atomic_load_acquire_int(&queue->items))
         return;
      /* rather than sleep, take work off the queue */
      if (queue->pool && tpool_help(queue->pool))
         continue;
      key = retro_eventcount_prepare_wait(&queue->done_ec);
      if (item ? retro_atomic_load_acquire_int(&item->done)
               : !retro_atomic_load_acquire_int(&queue->items))
      {
         retro_eventcount_cancel_wait(&queue->done_ec);
         return;
      }
      retro_eventcount_commit_wait(&queue->done_ec, key);
   }
}

osd_work_queue *osd_work_queue_alloc(int flags)
{
   int threads;
   int numprocs = osd_num_processors();
   osd_work_queue *queue = (osd_work_queue *)calloc(1, sizeof(*queue));

   if (!queue)
      return NULL;
   if (!retro_eventcount_init(&queue->done_ec))
   {
      free(queue);
      return NULL;
   }
   retro_mpsc_init(&queue->released);
   retro_atomic_int_init(&queue->items, 0);
   retro_atomic_int_init(&queue->slots, 0);
   queue->flags = flags;

   if (mosd_num_processors > 0)
      numprocs = MIN(4 * numprocs, mosd_num_processors);

   /* one processor: a thread for I/O queues only; otherwise all but one
    * for multi queues and one for the rest */
   if (numprocs <= 1)
      threads = (flags & WORK_QUEUE_FLAG_IO) ? 1 : 0;
   else
      threads = (flags & WORK_QUEUE_FLAG_MULTI) ? numprocs - 1 : 1;
   /* a thread that waits runs items too, under its own threadid */
   threads = MIN(threads, WORK_MAX_THREADS - 1);

   if (threads > 0)
      queue->pool = tpool_create(threads);
   return queue;
}

int osd_work_queue_items(osd_work_queue *queue)
{
   return retro_atomic_load_acquire_int(&queue->items);
}

int osd_work_queue_wait(osd_work_queue *queue, osd_ticks_t timeout)
{
   if (timeout)
      osd_work_wait_done(queue, NULL);
   return !retro_atomic_load_acquire_int(&queue->items);
}

void osd_work_queue_free(osd_work_queue *queue)
{
   if (!queue)
      return;
   if (queue->pool)
   {
      osd_work_wait_done(queue, NULL);
      /* joins the threads, so none is left inside done_ec */
      tpool_destroy(queue->pool);
   }
   retro_eventcount_free(&queue->done_ec);
   osd_work_item_list_free(queue->spare);
   osd_work_item_list_free(
         (osd_work_item *)retro_mpsc_take_all(&queue->released));
   free(queue);
}

osd_work_item *osd_work_item_queue_multiple(osd_work_queue *queue,
      osd_work_callback callback, int32_t numitems, void *parambase,
      int32_t paramstep, uint32_t flags)
{
   osd_work_item *item = NULL;
   int32_t itemnum;

   retro_atomic_fetch_add_int(&queue->items, numitems);

   for (itemnum = 0; itemnum < numitems; itemnum++)
   {
      if (!(item = osd_work_item_get(queue)))
      {
         retro_atomic_fetch_sub_int(&queue->items, numitems - itemnum);
         return NULL;
      }
      item->node.next = NULL;
      item->queue     = queue;
      item->callback  = callback;
      item->param     = parambase;
      item->result    = NULL;
      item->flags     = flags;
      retro_atomic_int_init(&item->done, 0);
      parambase       = (uint8_t *)parambase + paramstep;

      if (!queue->pool || !tpool_add_work(queue->pool, osd_work_item_run, item))
         osd_work_item_run(item);
   }

   /* an auto-release item may already be back on the free list */
   return (flags & WORK_ITEM_FLAG_AUTO_RELEASE) ? NULL : item;
}

int osd_work_item_wait(osd_work_item *item, osd_ticks_t timeout)
{
   if (timeout)
      osd_work_wait_done(item->queue, item);
   return retro_atomic_load_acquire_int(&item->done);
}

void *osd_work_item_result(osd_work_item *item)
{
   return item->result;
}

void osd_work_item_release(osd_work_item *item)
{
   osd_work_wait_done(item->queue, item);
   retro_mpsc_push(&item->queue->released, &item->node);
}

void osd_thread_cleanup(void)
{
   retro_eventcount_thread_cleanup();
}

#else /* !RETRO_OSD_THREADED */

/*-------------------------------------------------
    single-threaded fallback
-------------------------------------------------*/

struct _osd_work_item
{
   osd_work_item  *next;
   osd_work_queue *queue;
   void           *result;
};

struct _osd_work_queue
{
   osd_work_item *spare;
};

osd_lock *osd_lock_alloc(void)
{
   return (osd_lock *)1;
}

void osd_lock_acquire(osd_lock *lock) { }
int  osd_lock_try(osd_lock *lock) { return TRUE; }
void osd_lock_release(osd_lock *lock) { }
void osd_lock_free(osd_lock *lock) { }

osd_work_queue *osd_work_queue_alloc(int flags)
{
   return (osd_work_queue *)calloc(1, sizeof(osd_work_queue));
}

int osd_work_queue_items(osd_work_queue *queue)
{
   return 0;
}

int osd_work_queue_wait(osd_work_queue *queue, osd_ticks_t timeout)
{
   return TRUE;
}

void osd_work_queue_free(osd_work_queue *queue)
{
   if (!queue)
      return;
   while (queue->spare)
   {
      osd_work_item *next = queue->spare->next;
      free(queue->spare);
      queue->spare = next;
   }
   free(queue);
}

osd_work_item *osd_work_item_queue_multiple(osd_work_queue *queue,
      osd_work_callback callback, int32_t numitems, void *parambase,
      int32_t paramstep, uint32_t flags)
{
   void *result = NULL;
   osd_work_item *item;
   int32_t itemnum;

   for (itemnum = 0; itemnum < numitems; itemnum++)
   {
      result    = (*callback)(parambase, 0);
      parambase = (uint8_t *)parambase + paramstep;
   }
   if (flags & WORK_ITEM_FLAG_AUTO_RELEASE)
      return NULL;
   if ((item = queue->spare))
      queue->spare = item->next;
   else if (!(item = (osd_work_item *)malloc(sizeof(*item))))
      return NULL;
   item->queue  = queue;
   item->result = result;
   return item;
}

int osd_work_item_wait(osd_work_item *item, osd_ticks_t timeout)
{
   return TRUE;
}

void *osd_work_item_result(osd_work_item *item)
{
   return item->result;
}

void osd_work_item_release(osd_work_item *item)
{
   item->next          = item->queue->spare;
   item->queue->spare  = item;
}

void osd_thread_cleanup(void) { }

#endif /* RETRO_OSD_THREADED */
