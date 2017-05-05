/*
 * uvwrap.c
 *
 *  Created on: Apr 07, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#include <uvwrap.h>
#include <stdlib.h>

static void uvwrap__cb(siridb_handle_t * handle);
static void uvwrap__handle_cb(uv_async_t * async);

int uvwrap_init(siridb_t * conn, uv_loop_t * loop)
{
    conn->data = (void *) loop;
    conn->cb = uvwrap__cb;
    return 0;
}

int uvwrap_handle_init(siridb_handle_t * handle)
{
    uv_async_t * async = (uv_async_t *) malloc(sizeof(uv_async_t));
    if (async == NULL)
    {
        return -1;
    }

    async->data = (void *) handle->arg ;
    handle->arg = (void *) async;

    return 0;
}

static void uvwrap__cb(siridb_handle_t * handle)
{
    uv_async_t * async = (uv_async_t *) handle->arg;
    uv_loop_t * loop = (uv_loop_t *) handle->conn->data;

    handle->arg = async->data;
    async->data = (void *) handle;

    if (uv_async_init(loop, async, uvwrap__handle_cb) ||
        uv_async_send(async))
    {
        abort();
    }
}

static void uvwrap__handle_cb(uv_async_t * async)
{
    siridb_handle_t * handle = (siridb_handle_t *) async->data;
    handle->cb(handle);
    uv_close((uv_handle_t *) async, (uv_close_cb) free);
}
