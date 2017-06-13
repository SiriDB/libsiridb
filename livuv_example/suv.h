/*
 * suv.h - SiriDB C-Connector, example using libuv
 *
 *  Created on: Jun 09, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#ifndef SUV_H_
#define SUV_H_

#include <stdlib.h>
#include <libsiridb/siridb.h>
#include <uv.h>

/* type definitions */
typedef struct suv_buf_s suv_buf_t;
typedef struct suv_write_s suv_write_t;
typedef struct suv_write_s suv_connect_t;
typedef struct suv_write_s suv_query_t;
typedef struct suv_write_s suv_insert_t;

/* public functions */
suv_buf_t * suv_buf_create(siridb_t * siridb);
void suv_buf_destroy(suv_buf_t * suvbf);

suv_connect_t * suv_connect_create(
    siridb_req_t * req,
    const char * username,
    const char * password,
    const char * dbname);
void suv_connect_destroy(suv_connect_t * connect);
void suv_connect(
    suv_connect_t * connect,
    suv_buf_t * buf,
    uv_tcp_t * tcp,
    struct sockaddr * addr);

suv_query_t * suv_query_create(siridb_req_t * req, const char * query);
void suv_query_destroy(suv_query_t * suvq);
void suv_query(suv_query_t * suvq);

suv_insert_t * suv_insert_create(
    siridb_req_t * req,
    siridb_series_t * series[],
    size_t n);
void suv_insert_destroy(suv_insert_t * insert);
void suv_insert(suv_insert_t * insert);

/* struct definitions */
struct suv_buf_s
{
    char * buf;
    size_t len;
    size_t size;
    siridb_t * siridb;
};

struct suv_write_s
{
    void * data;            /* public */
    siridb_pkg_t * pkg;     /* packge to send */
    siridb_req_t * _req;    /* will not be cleared */
};

#endif /* SUV_H_ */