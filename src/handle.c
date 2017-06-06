/*
 * handle.c
 *
 *  Created on: May 04, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#include <siridb.h>
#include <stdlib.h>

siridb_handle_t * siridb_handle_create(siridb_t * siridb, siridb_cb cb)
{
    siridb_handle_t * handle;
    handle = (siridb_handle_t *) malloc(sizeof(siridb_handle_t));

    if (handle != NULL)
    {
        int rc;

        /* set next pid */
        handle->pid = siridb->pid++;
        handle->siridb = siridb;
        handle->cb = cb;
        handle->status = ERR_PENDING;
        handle->pkg = NULL;
        handle->data = NULL;

        rc = imap_add(siridb->imap, handle->pid, (void *) handle);

        if (rc == ERR_MEM_ALLOC)
        {
            free(handle);
            return NULL;
        }

        if (rc == ERR_OCCUPIED)
        {
            // error, a previous handle with this pid is found
            siridb_handle_t * prev;
            prev = (siridb_handle_t *) imap_pop(siridb->imap, handle->pid);

            assert (prev != NULL);

            prev->status = ERR_OVERWRITTEN;
            prev->cb(prev);

            rc = imap_add(siridb->imap, handle->pid, (void *) handle);
            if (rc == ERR_MEM_ALLOC)
            {
                free(handle);
                return NULL;
            }

            assert (rc == 0);
        }
    }

    return handle;
}

void siridb_handle_destroy(siridb_handle_t * handle)
{
    free(handle->pkg);
    free(handle);
}

void siridb_handle_cancel(siridb_handle_t * handle)
{
    /* remove handle from imap if exists */
    imap_pop(handle->conn->imap, (uint64_t) handle->pid);

    /* set err type and status */
    handle->status = ERR_CANCELLED;

    /* run callback */
    handle->cb(handle);
}
