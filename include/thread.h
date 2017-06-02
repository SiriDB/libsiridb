/*
 * thread.h
 *
 *  Created on: Apr 07, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */
#ifdef USE_THREAD

#ifndef SIRIDB_THREAD_H_
#define SIRIDB_THREAD_H_

#include <pthread.h>

typedef pthread_t siridb_thread_t;
typedef pthread_mutex_t siridb_mutex_t;

int siridb_thread_create(
        siridb_thread_t * tid,
        void (*entry)(void * arg),
        void *arg);

int siridb_mutex_init(siridb_mutex_t * mutex);
void siridb_mutex_destroy(siridb_mutex_t * mutex);
void siridb_mutex_lock(siridb_mutex_t * mutex);
void siridb_mutex_unlock(siridb_mutex_t * mutex);

#endif /* SIRIDB_THREAD_H_ */

#endif /* USE_THREAD */