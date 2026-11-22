#ifndef __RETROOS_H__
#define __RETROOS_H__

#define SDLMAME_UNIX

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
    osd_num_processors: return the number of processors

    Parameters:

        None.

    Return value:

        Number of processors
-----------------------------------------------------------------------------*/

int osd_num_processors(void);

/*-----------------------------------------------------------------------------
    osd_getenv: return pointer to environment variable

    Parameters:

        name  - name of environment variable

    Return value:

        pointer to value
-----------------------------------------------------------------------------*/
char *osd_getenv(const char *name);

/*-----------------------------------------------------------------------------
    osd_setenv: set environment variable

    Parameters:

        name  - name of environment variable
        value - value to write
        overwrite - overwrite if it exists

    Return value:

        0 on success
-----------------------------------------------------------------------------*/

int osd_setenv(const char *name, const char *value, int overwrite);

/*-----------------------------------------------------------------------------
    osd_thread_cleanup: release what the calling thread's waits on locks
    and work queues cached, before the core is unloaded
-----------------------------------------------------------------------------*/

void osd_thread_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif
