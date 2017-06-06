/*
 * siridbuv.c
 *
 *  Created on: Jun 02, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#include <conn.h>
#include <errmap.h>
#include <siridbuv.h>
#include <assert.h>
#include <stdlib.h>
#include <packer.h>
#include <string.h>
#include <protomap.h>
#include <siridb.h>

struct suv_s
{
    uv_tcp_t tcp;
};

static void siridb__uv_on_connect(uv_connect_t * req, int status);
static void siridb__uv_write_cb(uv_write_t * req, int status);

void siridb_uv_connect(
    siridb_handle_t * handle,
    uv_loop_t * loop,
    const char * username,
    const char * password,
    const char * dbname,
    const struct sockaddr * addr)
{
    /* only call siridb_uv_connect() with a handle without connection */
    assert (handle->siridb == NULL);

    handle->siridb = siridb_create();

    if (handle->siridb == NULL)
    {
        handle->status = ERR_MEM_ALLOC;
        handle->cb(handle);
        return;
    }

    struct suv_s * suv = (struct suv_s *) malloc(sizeof(struct suv_s));
    handle->conn->libconn = (void *) suv;

    if (suv == NULL)
    {
        handle->status = ERR_MEM_ALLOC;
        handle->cb(handle);
        return;
    }

    suv->tcp.data = (void *) handle->conn;

    uv_connect_t * req = (uv_connect_t *) malloc(sizeof(uv_connect_t));
    if (req == NULL)
    {
        handle->status = ERR_MEM_ALLOC;
        handle->cb(handle);
        return;
    }
    req->data = handle;

    uv_tcp_init(loop, &suv->tcp);
    uv_tcp_connect(
        req,
        &suv->tcp,
        addr,
        siridb__uv_on_connect);
}

const char * siridb_uv_strerror(siridb_handle_t * handle)
{
    switch (handle->errtp)
    {
        case ERRTP_DEF: return siridb_strerror(handle->status);
        case ERRTP_UV: return uv_strerror(handle->status);
    }
}


void siridb_uv_destroy(siridb_conn_t * conn)
{
    free(conn->libconn);
    siridb__conn_destroy(conn);
}

void siridb__write(siridb_handle_t * handle, siridb_pkg_t * pkg)
{
    uv_write_t * req = (uv_write_t *) malloc(sizeof(uv_write_t));
    struct suv_s * suv = (struct suv_s *) handle->conn->libconn;

    if (req == NULL)
    {
        siridb_handle_cancel(handle, ERRTP_DEF, ERR_MEM_ALLOC);
        return;
    }

    req->data = handle;

    uv_buf_t wrbuf = uv_buf_init(
            (char *) pkg,
            sizeof(siridb_pkg_t) + pkg->len);

    uv_write(
            req,
            (uv_stream_t *) &suv->tcp,
            &wrbuf,
            1,
            siridb__uv_write_cb);
}

static void siridb__uv_on_connect(uv_connect_t * req, int status)
{
    siridb_handle_t * handle = (siridb_handle_t *) req->data;
    siridb_conn_t * conn = handle->conn;
    struct suv_s * suv = (struct suv_s *) conn->data;

    if (status == 0)
    {
        siridb_pkg_t * pkg = siridb_pkg_auth(;
        siridb_packer_t * packer;

        siridb_pkg_auth

        // Connection created, sending auth request
        if ((packer = siridb_packer_create(512)) == NULL ||
            qp_add_array(&packer) ||
            qp_add_raw(packer, conn->username, strlen(conn->username)) ||
            qp_add_raw(packer, conn->password, strlen(conn->password)) ||
            qp_add_raw(packer, conn->dbname, strlen(conn->dbname)) ||
            qp_close_array(packer))
        {
            handle->status = ERR_MEM_ALLOC;
            handle->cb(handle);
        }

        pkg = siridb_packer_2pkg(packer, CprotoReqAuth);

        uv_read_start(
                req->handle,
                sirinet_socket_alloc_buffer,
                sirinet_socket_on_data);

        siridb_send(handle, pkg);
    }
    else
    {
        handle->errtp = ERRTP_UV;
        handle->status = status;
        handle->cb(handle);
    }
    free(req);
}

static void siridb__uv_write_cb(uv_write_t * req, int status)
{
    if (status)
    {
        siridb_handle_cancel((siridb_handle_t *) req->data, ERRTP_UV, status);
    }
    free(req);
}


static void siridb__uv_alloc_buffer(
        uv_handle_t * handle,
        size_t suggested_size,
        uv_buf_t * buf)
{
    siridb_t * siridb = handle->data;
}