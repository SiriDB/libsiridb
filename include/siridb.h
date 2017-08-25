/*
 * siridb.h
 *
 *  Created on: Apr 07, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#ifndef SIRIDB_H_
#define SIRIDB_H_

#include <inttypes.h>
#include "queue.h"
#include "pkg.h"
#include "req.h"
#include "errmap.h"
#include "protomap.h"
#include "series.h"
#include "resp.h"
#include "packer.h"
#include "point.h"
#include "queue.h"
#include "version.h"

/* type definitions */
typedef struct siridb_s siridb_t;
typedef struct siridb_req_s siridb_req_t;
typedef void (*siridb_cb) (siridb_req_t * req);

/* public functions */
#ifdef __cplusplus
extern "C" {
#endif

siridb_t * siridb_create(void);
void siridb_destroy(siridb_t * siridb);
int siridb_on_pkg(siridb_t * siridb, siridb_pkg_t * pkg);
size_t siridb_queue_len(siridb_t * siridb);

#ifdef __cplusplus
}
#endif

/* struct definitions */
struct siridb_s
{
    void * data;        /* public */
    uint16_t pid;
    queue_t * queue;
};

#endif /* SIRIDB_H_ */
