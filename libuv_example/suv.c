/*
 * suv.c - SiriDB C-Connector libuv example
 *
 *  Created on: Jun 09, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#include "suv.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static suv_write_t * suv__write_create(void);
static void suv__write(suv_write_t * swrite);
static void suv__alloc_buf(uv_handle_t * handle, size_t sugsz, uv_buf_t * buf);
static void suv__on_data(uv_stream_t * clnt, ssize_t n, const uv_buf_t * buf);
static void suv__write_cb(uv_write_t * uvreq, int status);
static void suv__connect_cb(uv_connect_t * uvreq, int status);

const long int MAX_PKG_SIZE = 209715200; // can be changed to anything you want

/*
 * Create and returns a buffer object or NULL in case of an allocation error.
 */
suv_buf_t * suv_buf_create(siridb_t * siridb)
{
    suv_buf_t * suvbf = (suv_buf_t *) malloc(sizeof(suv_buf_t));
    if (suvbf != NULL)
    {
        suvbf->siridb = siridb;
        suvbf->len = 0;
        suvbf->size = 0;
        suvbf->buf = NULL;
    }
    return suvbf;
}

/*
 * Destroy a buffer object.
 */
void suv_buf_destroy(suv_buf_t * suvbf)
{
    free(suvbf->buf);
    free(suvbf);
}

/*
 * Create and return an connect object or NULL in case of an allocation error.
 */
suv_connect_t * suv_connect_create(
    siridb_req_t * req,
    const char * username,
    const char * password,
    const char * dbname)
{
    assert (req->data == NULL); /* req->data should be set to -this- */

    suv_write_t * connect = suv__write_create();
    if (connect != NULL)
    {
        connect->pkg = siridb_pkg_auth(req->pid, username, password, dbname);
        connect->_req = req;
        if (connect->pkg == NULL)
        {
            suv_write_destroy(connect);
            connect = NULL;
        }
    }
    return (suv_connect_t *) connect;
}

/*
 * Destroy an connect object.
 */
void suv_connect_destroy(suv_connect_t * connect)
{
    suv_write_destroy((suv_write_t * ) connect);
}

/*
 * Use this function to connect to SiriDB. Always use the callback defined by
 * the request object parsed to suv_connect_create() for errors.
 */
void suv_connect(
    suv_connect_t * connect,
    suv_buf_t * buf,
    uv_tcp_t * tcp,
    struct sockaddr * addr)
{
    assert (connect->_req->data == connect);  /* bind connect to req->data */

    uv_connect_t * uvreq = (uv_connect_t *) malloc(sizeof(uv_connect_t));
    if (uvreq == NULL)
    {
        suv_write_error((suv_write_t *) connect, ERR_MEM_ALLOC);
    }
    else
    {
        tcp->data = (void *) buf;
        uvreq->data = (void *) connect->_req;
        uv_tcp_connect(uvreq, tcp, addr, suv__connect_cb);
    }
}

/*
 * Create and return a query object or NULL in case of an allocation error.
 */
suv_query_t * suv_query_create(siridb_req_t * req, const char * query)
{
    assert (req->data == NULL); /* req->data should be set to -this- */

    suv_write_t * suvq = suv__write_create();
    if (suvq != NULL)
    {
        suvq->pkg = siridb_pkg_query(req->pid, query);
        suvq->_req = req;
        if (suvq->pkg == NULL)
        {
            suv_write_destroy(suvq);
            suvq = NULL;
        }
    }
    return (suv_query_t *) suvq;
}

/*
 * Destroy a query object.
 */
void suv_query_destroy(suv_query_t * suvq)
{
    suv_write_destroy((suv_write_t * ) suvq);
}

/*
 * This function actually runs a query. Always use the callback defined by
 * the request object parsed to suv_query_create() for errors.
 */
void suv_query(suv_query_t * suvq)
{
    suv__write((suv_write_t *) suvq);
}

/*
 * Create and return a insert object or NULL in case of an allocation error.
 */
suv_insert_t * suv_insert_create(
    siridb_req_t * req,
    siridb_series_t * series[],
    size_t n)
{
    assert (req->data == NULL); /* req->data should be set to -this- */

    suv_write_t * insert = suv__write_create();
    if (insert != NULL)
    {
        insert->pkg = siridb_pkg_series(req->pid, series, n);
        insert->_req = req;
        if (insert->pkg == NULL)
        {
            suv_write_destroy(insert);
            insert = NULL;
        }
    }
    return (suv_insert_t *) insert;
}

/*
 * Destroy a insert object.
 */
void suv_insert_destroy(suv_insert_t * insert)
{
    suv_write_destroy((suv_write_t * ) insert);
}

/*
 * This function actually send the insert. Always use the callback defined by
 * the request object parsed to suv_insert_create() for errors.
 */
void suv_insert(suv_insert_t * insert)
{
    suv__write((suv_write_t *) insert);
}

/*
 * Create and return a write object or NULL in case of an allocation error.
 */
static suv_write_t * suv__write_create(void)
{
    suv_write_t * swrite = (suv_write_t *) malloc(sizeof(suv_write_t));
    if (swrite != NULL)
    {
        swrite->data = NULL;
        swrite->pkg = NULL;
    }
    return swrite;
}

/*
 * Destroy a write object.
 */
void suv_write_destroy(suv_write_t * swrite)
{
    free(swrite->pkg);
    free(swrite);
}

/*
 * Set request error and run callback
 */
void suv_write_error(suv_write_t * swrite, int err_code)
{
    queue_pop(swrite->_req->siridb->queue, swrite->pkg->pid);
    swrite->_req->status = ERR_SOCK_WRITE;
    swrite->_req->cb(swrite->_req);
}

/*
 * This function actually send the data.
 */
static void suv__write(suv_write_t * swrite)
{
    assert (swrite->_req->data == swrite); /* bind swrite to req->data */

    uv_write_t * uvreq = (uv_write_t *) malloc(sizeof(uv_write_t));
    uv_stream_t * stream = (uv_stream_t *) swrite->_req->siridb->data;

    if (uvreq == NULL)
    {
        suv_write_error(swrite, ERR_MEM_ALLOC);
    }
    else
    {
        uvreq->data = (void *) swrite->_req;

        uv_buf_t buf = uv_buf_init(
            (char *) swrite->pkg,
            sizeof(siridb_pkg_t) + swrite->pkg->len);

        uv_write(uvreq, stream, &buf, 1, suv__write_cb);
    }
}


static void suv__connect_cb(uv_connect_t * uvreq, int status)
{
    siridb_req_t * req = (siridb_req_t *) uvreq->data;
    suv_connect_t * connect = (suv_connect_t *) req->data;

    if (status != 0)
    {
        /* error handling */
        printf("cannot create connection: %s\n",  uv_strerror(status));
        suv_write_error((suv_write_t *) connect, ERR_SOCK_CONNECT);
    }
    else
    {
        suv_buf_t * suvbf = (suv_buf_t *) uvreq->handle->data;

        /* bind uv_stream_t * to siridb->data */
        suvbf->siridb->data = (void *) uvreq->handle;

        uv_write_t * uvw = (uv_write_t *) malloc(sizeof(uv_write_t));
        if (uvw == NULL)
        {
            suv_write_error((suv_write_t *) connect, ERR_MEM_ALLOC);
        }
        else
        {
            uvw->data = (void *) req;

            uv_buf_t buf = uv_buf_init(
                    (char *) connect->pkg,
                    sizeof(siridb_pkg_t) + connect->pkg->len);

            uv_read_start(uvreq->handle, suv__alloc_buf, suv__on_data);
            uv_write(uvw, uvreq->handle, &buf, 1, suv__write_cb);
        }
    }
    free(uvreq);
}

static void suv__alloc_buf(uv_handle_t * handle, size_t sugsz, uv_buf_t * buf)
{
    suv_buf_t * suvbf = (suv_buf_t *) handle->data;
    if (suvbf->len == 0 && suvbf->size != sugsz)
    {
        free(suvbf->buf);
        suvbf->buf = (char *) malloc(sugsz);
        if (suvbf->buf == NULL)
        {
            abort(); /* memory allocation error */
        }
        suvbf->size = sugsz;
        suvbf->len = 0;
    }

    buf->base = suvbf->buf + suvbf->len;
    buf->len = suvbf->size - suvbf->len;
}

static void suv__on_data(uv_stream_t * clnt, ssize_t n, const uv_buf_t * buf)
{
    suv_buf_t * suvbf = (suv_buf_t *) clnt->data;
    siridb_pkg_t * pkg;
    size_t total_sz;
    int rc;

    if (n < 0)
    {
        if (n != UV_EOF)
        {
            printf("read error: %s\n", uv_err_name(n));
        }

        /* cleanup */
        uv_close((uv_handle_t *) clnt, NULL);
        return;
    }

    suvbf->len += n;

    if (suvbf->len < sizeof(siridb_pkg_t))
    {
        return;
    }

    pkg = (siridb_pkg_t *) suvbf->buf;
    if (!siridb_pkg_check_bit(pkg) || pkg->len > MAX_PKG_SIZE)
    {
        /* invalid package, close connection */
        printf("got an invalid package, closing connecition\n");

        // suv_buf_destroy(suvbf);
        uv_close((uv_handle_t *) clnt, NULL);
        return;
    }

    total_sz = sizeof(siridb_pkg_t) + pkg->len;

    if (suvbf->len < total_sz)
    {
        if (suvbf->size < total_sz)
        {
            char * tmp = realloc(suvbf->buf, total_sz);
            if (tmp == NULL)
            {
                abort(); /* memory allocation error */
            }
            suvbf->buf = tmp;
            suvbf->size = total_sz;
        }
        return;
    }

    if ((rc = siridb_on_pkg(suvbf->siridb, pkg)))
    {
        printf("error: %s\n", siridb_strerror(rc));
    }

    suvbf->len -= total_sz;

    if (suvbf->len > 0)
    {
        /* move data and call suv_on_data() function again */
        memmove(suvbf->buf, suvbf->buf + total_sz, suvbf->len);
        suv__on_data(clnt, 0, buf);
    }
}

static void suv__write_cb(uv_write_t * uvreq, int status)
{
    if (status)
    {
        /* error handling */
        siridb_req_t * req = (siridb_req_t *) uvreq->data;
        suv_write_t * swrite = (suv_write_t *) req->data;

        printf("cannot write to socket: %s\n",  uv_strerror(status));
        suv_write_error(swrite, ERR_SOCK_WRITE);
    }

    /* free uv_write_t */
    free(uvreq);
}


