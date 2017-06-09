/*
 * suv.c - SiriDB C-Connector libuv example
 *
 *  Created on: Jun 09, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#include "suv.h"

static void suv_write_cb(uv_write_t * uvreq, int status);
static void suv_alloc_buf(uv_handle_t * handle, size_t sug_sz, uv_buf_t * buf);
static void suv_on_data(uv_stream_t * client, ssize_t n, const uv_buf_t * buf);


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

void suv_buf_destroy(suv_buf_t * suvbf)
{
    free(suvbf->buf);
    free(suvbf);
}

suv_auth_t * suv_auth_create(const char * usr, * pwd, * dbname, siridb_cb cb)
{
    suv_auth_t * auth = (suv_auth_t *) malloc(sizeof(suv_auth_t));
    if (auth != NULL)
    {
        auth->username = strdup(usr);
        auth->password = strdup(pwd);
        auth->dbname = strdup(dbname);
        auth->cb = cb;
        if (auth->username == NULL ||
            auth->password == NULL ||
            auth->dbname == NULL)
        {
            suv_auth_destroy(auth);
            auth = NULL;
        }
    }
    return auth;
}

void suv_auth_destroy(suv_auth_t * auth)
{
    free(auth->username);
    free(auth->password);
    free(auth->dbname);
    free(auth);
}

void suv_connect_cb(uv_connect_t * uvreq, int status)
{
    if (status != 0)
    {
        /* error handling */
        printf("cannot create connection: %s\n",  uv_strerror(status));
    }
    else
    {
        uv_read_start(uvreq->handle, suv_alloc_buffer, suv_on_data);

        suv_buf_t * suvbf = (suv_buf_t *) uvreq->handle->data;

        siridb_req_t * req = siridb_req_create(suvbf->siridb, auth_cb, NULL);
        /* TODO: handle req == NULL or use rc code */

        siridb_pkg_t * pkg = siridb_pkg_auth(req->pid, USER, PASSWD, DBNAME);
        /* TODO: handle pkg == NULL */

        uv_write_t * uvw = (uv_write_t *) malloc(sizeof(uv_write_t));
        /* TODO: handle uvw == NULL */

        /* bind pkg to req->data and req to uvw->data so we can free
         * pkg when write is done or call callback functoin on errors.
         */
        req->data = (void *) pkg;
        uvw->data = (void *) req;

        uv_buf_t buf = uv_buf_init(
                (char *) pkg,
                sizeof(siridb_pkg_t) + pkg->len);


        uv_write(uvw, (uv_stream_t *) uvreq->handle, &buf, 1, suv_write_cb);
    }
    free(uvreq);
}

static void suv_alloc_buf(uv_handle_t * handle, size_t sug_sz, uv_buf_t * buf)
{
    suv_buf_t * suvbf = (suv_buf_t *) handle->data;
    if (suvbf->len == 0 && suvbf->size != sug_sz)
    {
        free(suvbf->buf);
        suvbf->buf = (char *) malloc(sug_sz);
        /* TODO: handle suvbf->buf == NULL */
        suvbf->size = sug_sz;
        suvbf->len = 0;
    }

    buf->base = suvbf->buf + suvbf->len;
    buf->len = suvbf->size - suvbf->len;
}

static void suv_on_data(uv_stream_t * client, ssize_t n, const uv_buf_t * buf)
{
    suv_buf_t * suvbf = (suv_buf_t *) client->data;
    siridb_pkg_t * pkg;
    size_t total_sz;

    if (n < 0)
    {
        if (n != UV_EOF)
        {
            printf("read error: %s\n", uv_err_name(n));
        }

        /* cleanup */
        siridb_suvbuf_destroy(suvbf);
        uv_close((uv_handle_t *) client, NULL);
        return;
    }

    suvbf->len += n;

    if (suvbf->len < sizeof(siridb_pkg_t))
    {
        return;
    }

    pkg = (siridb_pkg_t *) suvbf->buf;
    if (!siridb_pkg_check_bit(pkg))
    {
        /* TODO: handle invalid pkg */
    }

    total_sz = sizeof(siridb_pkg_t) + pkg->len;

    if (suvbf->len < total_sz)
    {
        if (suvbf->size < total_sz)
        {
            char * tmp = realloc(suvbf->buf, total_sz);
            /* TODO: handle tmp == NULL */
            suvbf->buf = tmp;
            suvbf->size = total_sz;
        }
        return;
    }

    siridb_on_pkg(suvbf->siridb, pkg);

    suvbf->len -= total_sz;

    if (suvbf->len > 0)
    {
        /* move data and call on_data() function again */
        memmove(suvbf->buf, suvbf->buf + total_sz, suvbf->len);
        on_data(client, 0, buf);
    }
}

static void suv_write_cb(uv_write_t * uvreq, int status)
{
    siridb_req_t * req = (siridb_req_t *) uvreq->data;
    if (status)
    {
        /* error handling */
        printf("cannot write to socket: %s\n",  uv_strerror(status));

        /* remove request from queue */
        queue_pop(req->siridb->queue, pkg->pid);
        req->status = ERR_SOCK_WRITE;
        req->cb(req);
    }
    /* free siridb_pkg_t */
    free(req->data);

    /* free uv_write_t */
    free(uvreq);
}

