/*
 * handle.c
 *
 *  Created on: May 04, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#include <siridb.h>
#include <stdlib.h>

void siridb_handle_init(
        siridb_handle_t * handle,
        siridb_t * conn,
        siridb_cb cb,
        void * arg)
{
    handle->conn = conn;
    handle->cb = cb;
    handle->arg = arg;
    handle->status = ERR_UNFINISHED;
    handle->pkg = NULL;
}

void siridb_handle_destroy(siridb_handle_t * handle)
{
    handle->status = ERR_DESTROYED;
    free(handle->pkg);
}

void siridb_handle_cancel(siridb_t * conn, siridb_handle_t * handle)
{
    /* remove handle from imap if exists */
    imap_pop(conn->imap, (uint64_t) handle->pid);

    /* cancel handle */
    siridb__handle_cancel(handle);
}

void siridb__handle_cancel(siridb_handle_t * handle)
{
    handle->status = ERR_CANCELLED;
    handle->cb(handle);
}
