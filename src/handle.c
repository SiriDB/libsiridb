/*
 * handle.c
 *
 *  Created on: May 04, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#include <siridb.h>
#include <stdlib.h>

siridb_handle_t * siridb_handle_create(
        siridb_conn_t * conn,
        siridb_cb cb,
        void * arg)
{
    siridb_handle_t * handle;
    handle = (siridb_handle_t *) malloc(sizeof(siridb_handle_t));
    if (handle != NULL)
    {
        handle->conn = conn;
        handle->cb = cb;
        handle->arg = arg;
        handle->status = ERR_NOT_FINISHED;
        handle->pkg = NULL;
    }
    return handle;
}

void siridb_handle_destroy(siridb_handle_t * handle)
{
    free(handle->pkg);
    free(handle);
}

void siridb_handle_cancel(siridb_conn_t * conn, siridb_handle_t * handle)
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
