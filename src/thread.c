/*
 * thread.c
 *
 *  Created on: Apr 07, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#include <thread.h>
#include <stdlib.h>

int siridb_thread_create(
        siridb_thread_t * tid,
        void (*entry)(void * arg),
        void *arg)
{
    return -pthread_create(tid, NULL, (void*(*)(void*)) entry, arg);
}

int siridb_mutex_init(siridb_mutex_t * mutex)
{
#if defined(NDEBUG) || !defined(PTHREAD_MUTEX_ERRORCHECK)
    return -pthread_mutex_init(mutex, NULL);
#else
    pthread_mutexattr_t attr;
    int err;

    if (pthread_mutexattr_init(&attr))
    {
        abort();
    }

    if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK))
    {
        abort();
    }

    err = pthread_mutex_init(mutex, &attr);

    if (pthread_mutexattr_destroy(&attr))
    {
        abort();
    }

    return -err;
#endif
}

void siridb_mutex_destroy(siridb_mutex_t * mutex)
{
    if (pthread_mutex_destroy(mutex))
    {
        abort();
    }
}

void siridb_mutex_lock(siridb_mutex_t * mutex)
{
    if (pthread_mutex_lock(mutex))
    {
        abort();
    }
}

void siridb_mutex_unlock(siridb_mutex_t * mutex)
{
    if (pthread_mutex_unlock(mutex))
    {
        abort();
    }
}
