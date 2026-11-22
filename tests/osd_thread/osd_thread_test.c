/* osd_thread_test.c - stress test for the OSD locks and work queues in
 * src/osd/retro/retrothread.c.
 *
 * Checks that every queued item runs exactly once, that no two callbacks
 * run at once under the same threadid, that threadids stay below
 * WORK_MAX_THREADS, that non-auto-release items report their result
 * through wait/result/release, that freeing a queue runs what is still
 * queued, and that osd_lock excludes other threads while letting its
 * holder re-enter.  Exits non-zero on the first failure.
 */

#include <stdarg.h>
#include <stdio.h>
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
#undef INLINE

#include <retro_atomic.h>
#ifdef HAVE_THREADS
#include <rthreads/rthreads.h>
#define TEST_LOCKERS    4
#else
#define TEST_LOCKERS    1
#endif

extern int mosd_num_processors;

#define TEST_PROCESSORS 4
#define TEST_ITEMS      257
#define TEST_ROUNDS     400
#define TEST_IO_ITEMS   20000
#define TEST_LOCK_ITERS 200000

/* what retrothread.c takes from the rest of the OSD */
int osd_num_processors(void)
{
   return TEST_PROCESSORS;
}

osd_ticks_t osd_ticks_per_second(void)
{
   return 1000000;
}

static int failures = 0;

static void CHECK(int cond, const char *fmt, ...)
{
   va_list ap;
   if (cond)
      return;
   va_start(ap, fmt);
   fputs("FAIL: ", stderr);
   vfprintf(stderr, fmt, ap);
   fputc('\n', stderr);
   va_end(ap);
   failures++;
}

/* one per threadid: 1 while a callback holds it */
static retro_atomic_int_t slot_busy[WORK_MAX_THREADS];
static retro_atomic_int_t slot_clash;
static retro_atomic_int_t slot_range;

typedef struct test_unit
{
   retro_atomic_int_t runs;
   int                pad[15];
} test_unit;

static test_unit units[TEST_ITEMS];

static void *unit_callback(void *param, int threadid)
{
   test_unit *unit = (test_unit *)param;
   volatile int spin;

   if (threadid < 0 || threadid >= WORK_MAX_THREADS)
   {
      retro_atomic_fetch_add_int(&slot_range, 1);
      return NULL;
   }
   if (retro_atomic_exchange_int(&slot_busy[threadid], 1) != 0)
      retro_atomic_fetch_add_int(&slot_clash, 1);
   /* long enough for callbacks on other threads to overlap this one */
   for (spin = 0; spin < 200; spin++) { }
   retro_atomic_fetch_add_int(&unit->runs, 1);
   retro_atomic_store_release_int(&slot_busy[threadid], 0);
   return NULL;
}

static void test_multi_queue(int processors)
{
   int round, i;
   osd_work_queue *queue;

   mosd_num_processors = processors;
   queue = osd_work_queue_alloc(WORK_QUEUE_FLAG_MULTI | WORK_QUEUE_FLAG_HIGH_FREQ);
   CHECK(queue != NULL, "multi queue alloc");
   if (!queue)
      return;

   for (i = 0; i < TEST_ITEMS; i++)
      retro_atomic_int_init(&units[i].runs, 0);

   for (round = 1; round <= TEST_ROUNDS; round++)
   {
      /* batches of different sizes, as poly flushes them */
      int first = 0;
      while (first < TEST_ITEMS)
      {
         int count = 1 + (round * 7 + first) % 64;
         if (count > TEST_ITEMS - first)
            count = TEST_ITEMS - first;
         CHECK(osd_work_item_queue_multiple(queue, unit_callback, count,
                  &units[first], sizeof(units[0]),
                  WORK_ITEM_FLAG_AUTO_RELEASE) == NULL,
               "auto-release batch returned an item");
         first += count;
      }
      CHECK(osd_work_queue_wait(queue, osd_ticks_per_second() * 10),
            "multi queue wait timed out in round %d", round);
      CHECK(osd_work_queue_items(queue) == 0, "items left after wait");
      for (i = 0; i < TEST_ITEMS; i++)
         if (retro_atomic_load_acquire_int(&units[i].runs) != round)
         {
            CHECK(0, "unit %d ran %d times by round %d (processors %d)", i,
                  retro_atomic_load_acquire_int(&units[i].runs), round,
                  processors);
            break;
         }
      if (failures)
         break;
   }
   osd_work_queue_free(queue);
}

static void *io_callback(void *param, int threadid)
{
   return (void *)((uintptr_t)param * 3 + 1);
}

static void test_io_queue(void)
{
   int i;
   osd_work_queue *queue;

   mosd_num_processors = 0;
   queue = osd_work_queue_alloc(WORK_QUEUE_FLAG_IO);
   CHECK(queue != NULL, "io queue alloc");
   if (!queue)
      return;
   for (i = 0; i < TEST_IO_ITEMS && !failures; i++)
   {
      osd_work_item *item = osd_work_item_queue(queue, io_callback,
            (void *)(uintptr_t)i, 0);
      CHECK(item != NULL, "io item %d not returned", i);
      if (!item)
         break;
      CHECK(osd_work_item_wait(item, osd_ticks_per_second() * 10),
            "io item %d wait failed", i);
      CHECK(osd_work_item_result(item) == (void *)((uintptr_t)i * 3 + 1),
            "io item %d result wrong", i);
      osd_work_item_release(item);
   }
   osd_work_queue_free(queue);
}

static void test_free_with_work_queued(void)
{
   int i;
   osd_work_queue *queue;

   mosd_num_processors = 0;
   queue = osd_work_queue_alloc(WORK_QUEUE_FLAG_MULTI);
   CHECK(queue != NULL, "queue alloc");
   if (!queue)
      return;
   for (i = 0; i < TEST_ITEMS; i++)
      retro_atomic_int_init(&units[i].runs, 0);
   osd_work_item_queue_multiple(queue, unit_callback, TEST_ITEMS, &units[0],
         sizeof(units[0]), WORK_ITEM_FLAG_AUTO_RELEASE);
   osd_work_queue_free(queue);
   for (i = 0; i < TEST_ITEMS; i++)
      CHECK(retro_atomic_load_acquire_int(&units[i].runs) == 1,
            "unit %d ran %d times across queue free", i,
            retro_atomic_load_acquire_int(&units[i].runs));
}

static osd_lock          *test_lock;
static int                lock_counter;
static retro_atomic_int_t lock_inside;
static retro_atomic_int_t lock_clash;

static void locker(void *arg)
{
   int i;
   for (i = 0; i < TEST_LOCK_ITERS; i++)
   {
      int nested = (i & 3) == 0;
      if ((i & 7) == 5)
      {
         if (!osd_lock_try(test_lock))
            continue;
      }
      else
         osd_lock_acquire(test_lock);
      if (retro_atomic_fetch_add_int(&lock_inside, 1) != 0)
         retro_atomic_fetch_add_int(&lock_clash, 1);
      if (nested)
         osd_lock_acquire(test_lock);
      lock_counter++;
      if (nested)
         osd_lock_release(test_lock);
      retro_atomic_fetch_sub_int(&lock_inside, 1);
      osd_lock_release(test_lock);
   }
   osd_thread_cleanup();
}

static void test_lock_exclusion(void)
{
   int i, expected = 0;
#ifdef HAVE_THREADS
   sthread_t *threads[TEST_LOCKERS];
#endif

   test_lock = osd_lock_alloc();
   CHECK(test_lock != NULL, "lock alloc");
   if (!test_lock)
      return;
   retro_atomic_int_init(&lock_inside, 0);
   retro_atomic_int_init(&lock_clash, 0);
#ifdef HAVE_THREADS
   for (i = 0; i < TEST_LOCKERS; i++)
      threads[i] = sthread_create(locker, NULL);
   for (i = 0; i < TEST_LOCKERS; i++)
   {
      CHECK(threads[i] != NULL, "locker %d not started", i);
      if (threads[i])
         sthread_join(threads[i]);
   }
#else
   locker(NULL);
#endif
   CHECK(retro_atomic_load_acquire_int(&lock_clash) == 0,
         "%d lock holders overlapped", retro_atomic_load_acquire_int(&lock_clash));
   /* lock_counter counts the tries that failed out, so only bound it */
   for (i = 0; i < TEST_LOCK_ITERS; i++)
      if ((i & 7) != 5)
         expected++;
   CHECK(lock_counter >= expected * TEST_LOCKERS
         && lock_counter <= TEST_LOCK_ITERS * TEST_LOCKERS,
         "lock counter %d outside [%d, %d]", lock_counter,
         expected * TEST_LOCKERS, TEST_LOCK_ITERS * TEST_LOCKERS);
   osd_lock_free(test_lock);
}

int main(void)
{
   int i;
   for (i = 0; i < WORK_MAX_THREADS; i++)
      retro_atomic_int_init(&slot_busy[i], 0);
   retro_atomic_int_init(&slot_clash, 0);
   retro_atomic_int_init(&slot_range, 0);

   test_multi_queue(0);   /* a thread per processor */
   test_multi_queue(1);   /* items run on the queueing thread */
   test_multi_queue(64);  /* more threads than slots would allow */
   test_io_queue();
   test_free_with_work_queued();
   test_lock_exclusion();

   CHECK(retro_atomic_load_acquire_int(&slot_clash) == 0,
         "%d callbacks shared a threadid", retro_atomic_load_acquire_int(&slot_clash));
   CHECK(retro_atomic_load_acquire_int(&slot_range) == 0,
         "%d callbacks got a threadid out of range",
         retro_atomic_load_acquire_int(&slot_range));
   osd_thread_cleanup();

   if (failures)
   {
      fprintf(stderr, "osd_thread_test: %d failure(s)\n", failures);
      return 1;
   }
   printf("osd_thread_test: ok\n");
   return 0;
}
