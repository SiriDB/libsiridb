/*
 * handle.h
 *
 *  Created on: May 04, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#ifndef SIRIDB_HANDLE_H_
#define SIRIDB_HANDLE_H_

typedef struct siridb_handle_s siridb_handle_t;
typedef struct siridb_s siridb_t;
typedef void (*siridb_cb) (siridb_handle_t * handle);

void siridb__handle_cancel(siridb_handle_t * handle);

struct siridb_handle_s
{
    siridb_t * conn;
    siridb_cb cb;
    void * arg;
    int status;
    siridb_pkg_t * pkg;
};

#endif /* SIRIDB_HANDLE_H_ */
