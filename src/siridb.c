/*
 * siridb.c
 *
 *  Created on: Apr 07, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#include <siridb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

siridb_t * siridb_create(void)
{
    siridb_t * siridb = (siridb_t *) malloc(sizeof(siridb_t));
    if (siridb != NULL)
    {
        siridb->data = NULL;
        siridb->pid = 0;
        siridb->queue = queue_create();

        if (siridb->queue == NULL)
        {
            free(siridb);
            siridb = NULL;
        }
    }
    return siridb;
}

void siridb_destroy(siridb_t * siridb)
{
    queue_walk(siridb->queue, (queue_cb) siridb__req_cancel);
    queue_destroy(siridb->queue);
    free(siridb);
}

int siridb_on_pkg(siridb_t * siridb, siridb_pkg_t * pkg)
{
    siridb_req_t * req =
            (siridb_req_t *) queue_pop(siridb->queue, (uint64_t) pkg->pid);

    if (req == NULL)
    {
        return ERR_NOT_FOUND;
    }

    req->pkg = siridb_pkg_dup(pkg);
    req->status = (req->pkg == NULL) ? ERR_MEM_ALLOC : 0;
    req->cb(req);

    return 0;
}
