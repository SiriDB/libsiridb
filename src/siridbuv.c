/*
 * siridbuv.c
 *
 *  Created on: Jun 02, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#include <uv.h>
#include <conn.h>

struct suv_s
{
    uv_tcp_t tcp;
    siridb_handle_t * handle;
}

static void siridb__uv_on_connect(uv_connect_t * req, int status);


void siridb_uv_connect(
    siridb_handle_t * handle,
    uv_loop_t loop,
    const char * username,
    const char * password,
    const char * dbname,
    const sockaddr * addr)
{
    siridb_conn_t * conn = siridb__conn_create(
        username,
        password,
        dbname);

    if (conn == NULL)
    {
        handle->status = ERR_MEM_ALLOC;
        handle->cb(handle);
        return;
    }

    conn->_conn = malloc(sizeof(struct suv_s));

    if (conn->_conn == NULL)
    {
        siridb_uv_destroy(conn);
        handle->status = ERR_MEM_ALLOC;
        handle->cb(handle);
        return;
    }

    conn->_conn
    conn->_conn->tcp.data = (void *) conn;

    uv_connect_t * req = (uv_connect_t *) malloc(sizeof(uv_connect_t));
    if (req == NULL)
    {
        siridb_uv_destroy(conn);
        return NULL;
    }

    uv_tcp_init(loop, &conn->_conn->tcp);


    uv_tcp_connect(req, &conn->_conn->tcp, addr, siridb__uv_on_connect);
}

void siridb_uv_destroy(siridb_conn_t * conn)
{
    free(conn->_conn);

    siridb__conn_destroy(conn);
}

static void siridb__uv_on_connect(uv_connect_t * req, int status)
{

}