/*
 * req.c
 *
 *  Created on: May 04, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#include <siridb.h>
#include <stdlib.h>

siridb_req_t * siridb_req_create(siridb_t * siridb, siridb_cb cb)
{
    siridb_req_t * req;
    req = (siridb_req_t *) malloc(sizeof(siridb_req_t));

    if (req != NULL)
    {
        int rc;

        /* set next pid */
        req->pid = siridb->pid++;
        req->siridb = siridb;
        req->cb = cb;
        req->status = ERR_PENDING;
        req->pkg = NULL;
        req->data = NULL;

        rc = imap_add(siridb->imap, req->pid, (void *) req);

        if (rc == ERR_MEM_ALLOC)
        {
            free(req);
            return NULL;
        }

        if (rc == ERR_OCCUPIED)
        {
            // error, a previous req with this pid is found
            siridb_req_t * prev;
            prev = (siridb_req_t *) imap_pop(siridb->imap, req->pid);

            assert (prev != NULL);

            prev->status = ERR_OVERWRITTEN;
            prev->cb(prev);

            rc = imap_add(siridb->imap, req->pid, (void *) req);
            if (rc == ERR_MEM_ALLOC)
            {
                free(req);
                return NULL;
            }

            assert (rc == 0);
        }
    }

    return req;
}

void siridb_req_destroy(siridb_req_t * req)
{
    free(req->pkg);
    free(req);
}

void siridb_req_cancel(siridb_req_t * req)
{
    /* remove req from imap if exists */
    imap_pop(req->conn->imap, (uint64_t) req->pid);

    /* set err type and status */
    req->status = ERR_CANCELLED;

    /* run callback */
    req->cb(req);
}
