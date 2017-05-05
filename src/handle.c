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

void siridb__handle_cancel(siridb_handle_t * handle)
{
    handle->status = ERR_CANCELLED;
    handle->cb(handle);
}
