/*
 * req.c
 *
 *  Created on: May 04, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#include <siridb.h>
#include <stdlib.h>
#include <assert.h>


siridb_req_t * siridb_req_create(siridb_t * siridb, siridb_cb cb, int * rc)
{
    int fallb;
    int * rcode = (rc == NULL) ? &fallb : rc;
    uint16_t opid = 0;
    siridb_req_t * req;

    req = (siridb_req_t *) malloc(sizeof(siridb_req_t));
    if (req == NULL)
    {
        *rcode = ERR_MEM_ALLOC;
        return NULL;
    }

    req->siridb = siridb;
    req->cb = cb;
    req->status = ERR_PENDING;
    req->pkg = NULL;
    req->data = NULL;

    /*
     * Try all pids, usually the first one fits but in a rare case this one
     * could be in use.
     */
    while (++opid)
    {
        req->pid = siridb->pid++;

        *rcode = queue_add(siridb->queue, req->pid, (void *) req);

        if (*rcode == ERR_MEM_ALLOC)
        {
            free(req);
            return NULL;
        }

        if (*rcode == 0)
        {
            return req;
        }

        /* position is occupied, try another... */
        assert (*rcode == ERR_OCCUPIED);
    }

    /* 2**16 rquests are pending, no place for another one... */
    free(req);
    return NULL;
}

void siridb_req_destroy(siridb_req_t * req)
{
    /* Status should never be ERR_PENDING when the callback function is called
     * and the callback function must be called before destroying the request.
     */
    assert (req->status != ERR_PENDING);

    free(req->pkg);
    free(req);
}

void siridb_req_cancel(siridb_req_t * req)
{
    /* remove req from queue if exists */
    queue_pop(req->siridb->queue, (uint64_t) req->pid);

    /* set err type and status */
    req->status = ERR_CANCELLED;

    /* run callback */
    req->cb(req);
}
