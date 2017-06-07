/*
 * req.h
 *
 *  Created on: May 04, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#ifndef SIRIDB_REQ_H_
#define SIRIDB_REQ_H_

#include <conn.h>
#include <pkg.h>

typedef struct siridb_req_s siridb_req_t;
typedef struct siridb_conn_s siridb_conn_t;
typedef void (*siridb_cb) (siridb_req_t * req);

siridb_req_t * siridb_req_create(siridb_t * siridb, siridb_cb cb);
void siridb_req_destroy(siridb_req_t * req);
void siridb_req_cancel(siridb_req_t * req);

struct siridb_req_s
{
    void * data;                /* public */
    uint16_t pid;
    int status;
    siridb_pkg_t * pkg;
    siridb_cb cb;
    siridb_t * siridb;
};

#endif /* SIRIDB_REQ_H_ */
