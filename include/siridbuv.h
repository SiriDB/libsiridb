/*
 * siridbuv.h
 *
 *  Created on: Jun 02, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */
#ifndef SIRIDB_SIRIDBUV_H_
#define SIRIDB_SIRIDBUV_H_

#include <uv.h>
#include <req.h>

typedef struct suv_s suv_t;

struct suv_handle_s
{
    void * data;  /* public */
    int err_code; /* uv error code */
}

struct suv_s
{
    uv_tcp_t tcp;
    char * buf;
    size_t len;
    size_t size;
}

const char * siridb_uv_strerror(siridb_handle_t * handle);
void siridb_uv_connect(
    siridb_handle_t * handle,
    uv_loop_t * loop,
    const char * username,
    const char * password,
    const char * dbname,
    const struct sockaddr * addr);

#endif /* SIRIDB_SIRIDBUV_H_ */
