/*
 * handle.h
 *
 *  Created on: May 04, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#ifndef SIRIDB_HANDLE_H_
#define SIRIDB_HANDLE_H_

#include <conn.h>
#include <pkg.h>

typedef struct siridb_handle_s siridb_handle_t;
typedef struct siridb_conn_s siridb_conn_t;
typedef void (*siridb_cb) (siridb_handle_t * handle);

siridb_handle_t * siridb_handle_create(siridb_t * siridb, siridb_cb cb);
void siridb_handle_destroy(siridb_handle_t * handle);
void siridb_handle_cancel(siridb_handle_t * handle);

struct siridb_handle_s
{
    void * data;                /* public */
    uint16_t pid;
    int status;
    siridb_pkg_t * pkg;
    siridb_cb cb;
    siridb_t * siridb;
};

#endif /* SIRIDB_HANDLE_H_ */
