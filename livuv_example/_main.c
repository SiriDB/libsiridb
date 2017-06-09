/*
 * main.c
 *
 *  Created on: Apr 07, 2017
 *      Author: joente
 */

#include <stdio.h>
#include <string.h>
#include <siridb.h>
#include <uv.h>
#include <stdlib.h>
#include <stdio.h>
#include <qpack.h>
#include <resp.h>

typedef struct suv_buf_s suv_buf_t;

struct suv_buf_s
{
    char * buf;
    size_t len;
    size_t size;
    siridb_t * siridb;
};

int results = 0;
uv_loop_t * loop;

void alloc_buffer(uv_handle_t * handle, size_t sug_sz, uv_buf_t * buf);
void on_data(uv_stream_t * client, ssize_t nread, const uv_buf_t * buf);

void wait_for_a_while(uv_idle_t* handle) {
    if (results >= 2)
        uv_idle_stop(handle);
}

static void print_val(qp_res_t * val)
{
    switch (val->tp)
    {
    case QP_RES_MAP:
        printf("{ ... }"); break;
    case QP_RES_ARRAY:
        printf("[ ... ]"); break;
    case QP_RES_INT64:
        printf("%ld", val->via.int64); break;
    case QP_RES_REAL:
        printf("%f", val->via.real); break;
    case QP_RES_STR:
        printf("%s", val->via.str); break;
    case QP_RES_BOOL:
        printf("%d", val->via.bool); break;
    case QP_RES_NULL:
        printf("null"); break;
    }
}

static void on_data2(siridb_req_t * req)
{
    printf("On Data 2...%d (pid: %u)\n", req->status, req->pkg->pid);
    if (req->status == 0)
    {
        int r;
        siridb_resp_t * resp = siridb_resp_create(req, &r);
        printf("Resp parse status: %d\n", r);
        if (resp->timeit != NULL)
        {
            printf("got timeit for server: %s with time: %f\n",
                    resp->timeit->perfs[0].server,
                    resp->timeit->perfs[0].time);
        }
        if (resp->tp == SIRIDB_RESP_TP_SELECT)
        {
            siridb_select_t * select = resp->via.select;
            printf("Select response for %lu series.\n", select->n);
            for (size_t m = 0; m < select->n; m++)
            {
                printf("Series: %s (points: %lu, type: %d)\n",
                        select->series[m]->name,
                        select->series[m]->n,
                        select->series[m]->tp);
            }
        }
        if (resp->tp == SIRIDB_RESP_TP_LIST)
        {
            siridb_list_t * list = resp->via.list;
            printf("List response with %lu columns and %lu rows...\n",
                    list->headers->via.array->n, list->data->via.array->n);
            for (size_t r = 0; r < list->data->via.array->n; r++)
            {
                qp_array_t * row = list->data->via.array->values[r].via.array;
                for (size_t c = 0; c < row->n; c++)
                {
                    print_val(row->values + c);
                    printf(", ");
                }
                printf("\n");
            }
        }
        if (resp->tp == SIRIDB_RESP_TP_COUNT)
        {
            printf("Count response: %lu\n", resp->via.count);
        }
        if (resp->tp == SIRIDB_RESP_TP_CALC)
        {
            printf("Calc response: %lu\n", resp->via.calc);
        }
        if (resp->tp == SIRIDB_RESP_TP_SHOW)
        {
            size_t i;
            siridb_show_t * show = resp->via.show;
            printf("Show response with %lu items\n", show->n);
            for (i = 0; i < show->n; i++)
            {
                printf("name: %s    value tp: ", show->items[i].key);
                print_val(show->items[i].value);
                printf("\n");
            }
        }
        if (resp->tp == SIRIDB_RESP_TP_ERROR_MSG)
        {
            printf("Error msg: %s\n", resp->via.error_msg);
        }
        if (resp->tp == SIRIDB_RESP_TP_SUCCESS_MSG)
        {
            printf("Success msg: %s\n", resp->via.success_msg);
        }
        if (resp->tp == SIRIDB_RESP_TP_ERROR)
        {
            printf("Error: %s\n", resp->via.error);
        }
        if (resp->tp == SIRIDB_RESP_TP_SUCCESS)
        {
            printf("Success: %s\n", resp->via.success);
        }
        siridb_resp_destroy(resp);
    }
    siridb_req_destroy(req);
    results++;

}

static void walk_close_handlers(uv_handle_t * handle, void * arg)
{

    if (uv_is_closing(handle))
    {
        return;
    }

    switch (handle->type)
    {
    case UV_WORK:
        printf("WORK!!\n");
        break;

    case UV_SIGNAL:
        uv_close(handle, NULL);
        break;

    case UV_TCP:
        printf("TCP!!\n");
        break;

    case UV_TIMER:
        printf("TIMER!!\n");
        break;

    case UV_ASYNC:
        printf("ASYNC!!\n");
        break;

    case UV_IDLE:
        uv_close(handle, NULL);
        break;

    default:
        printf("HANDLE TPYE: %d\n", handle->type);
        break;
    }
}

static void close_handlers(void)
{
    /* close open handlers */
    uv_walk(loop, walk_close_handlers, NULL);

    /* run the loop once more so call-backs on uv_close() can run */
    uv_run(loop, UV_RUN_DEFAULT);
}

static void auth_cb(siridb_req_t * req)
{
    if (!req->status)
}

const char * SERVER = "127.0.0.1";
const int PORT = 9000;
const char * USER = "iris";
const char * PASSWD = "siri";
const char * DBNAME = "dbtest";
const int SUGGESTED_BUF_SIZE = 65536;

static void write_cb(uv_write_t * uvreq, int status)
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


static void connect_cb(uv_connect_t * uvreq, int status)
{
    if (status != 0)
    {
        /* error handling */
        printf("cannot create connection: %s\n",  uv_strerror(status));
    }
    else
    {
        uv_read_start(uvreq->handle, alloc_buffer, on_data);

        suv_buf_t * suvbf = (suv_buf_t *) uvreq->handle->data;
        printf("Here.1..%p\n", suvbf);
        printf("Here.2..%p\n", suvbf->siridb);

        siridb_req_t * req = siridb_req_create(suvbf->siridb, auth_cb, NULL);
        /* check for not NULL or use rc code instead of NULL */

        siridb_pkg_t * pkg = siridb_pkg_auth(req->pid, USER, PASSWD, DBNAME);
        /* check for not NULL */

        printf("Here.3..%p\n", pkg);

        uv_write_t * authreq = (uv_write_t *) malloc(sizeof(uv_write_t));
        /* check for not NULL */

        printf("Here.4..%p\n", authreq);

        /* bind pkg to req->data and req to authreq->data so we can free
         * pkg when write is done or call callback functoin on errors.
         */
        req->data = (void *) pkg;
        authreq->data = (void *) req;

        uv_buf_t buf = uv_buf_init(
                (char *) pkg,
                sizeof(siridb_pkg_t) + pkg->len);

        printf("Here.5..%p\n", authreq);

        uv_write(authreq, (uv_stream_t *) uvreq->handle, &buf, 1, write_cb);
    }
    free(uvreq);
}

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

void alloc_buffer(uv_handle_t * handle, size_t sug_sz, uv_buf_t * buf)
{
    suv_buf_t * suvbf = (suv_buf_t *) handle->data;
    printf("Alloc buffer...\n");
    if (suvbf->len == 0 && suvbf->size != sug_sz)
    {
        free(suvbf->buf);
        suvbf->buf = (char *) malloc(sug_sz);
        suvbf->size = sug_sz;
        suvbf->len = 0;
    }

    buf->base = suvbf->buf + suvbf->len;
    buf->len = suvbf->size - suvbf->len;
}

void on_data(uv_stream_t * client, ssize_t nread, const uv_buf_t * buf)
{
    suv_buf_t * suvbf = (suv_buf_t *) client->data;
    siridb_pkg_t * pkg;
    size_t total_sz;

    if (nread < 0)
    {
        if (nread != UV_EOF)
        {
            printf("read error: %s\n", uv_err_name(nread));
        }

        /* cleanup */
        siridb_suvbuf_destroy(suvbf);
        uv_close((uv_handle_t *) client, NULL);
        return;
    }

    suvbf->len += nread;

    if (suvbf->len < sizeof(siridb_pkg_t))
    {
        return;
    }

    printf("Got data...\n");

    pkg = (siridb_pkg_t *) suvbf->buf;
    if (!siridb_pkg_check_bit(pkg))
    {
        /* handle invalid pkg */
    }

    total_sz = sizeof(siridb_pkg_t) + pkg->len;

    if (suvbf->len < total_sz)
    {
        if (suvbf->size < total_sz)
        {
            char * tmp = realloc(suvbf->buf, total_sz);
            /* handle tmp == NULL */
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

int _main(void)
{
    uv_tcp_t tcp;
    struct sockaddr_in addr;
    uv_idle_t idler;

    loop = (uv_loop_t *) malloc(sizeof(uv_loop_t));
    /* check for not NULL */
    uv_loop_init(loop);

    uv_idle_init(loop, &idler);
    uv_idle_start(&idler, wait_for_a_while);

    uv_ip4_addr(SERVER, PORT, &addr);

    siridb_t * siridb = siridb_create();
    /* handle siridb == NULL */

    suv_buf_t * suvbf = suv_buf_create(siridb);
    /* handle suvbf == NULL */

    uv_connect_t * uvreq = (uv_connect_t *) malloc(sizeof(uv_connect_t));
    /* handle uvreq == NULL */

    tcp.data = (void *) suvbf;

    uv_tcp_init(loop, &tcp);
    uv_tcp_connect(uvreq, &tcp, (struct sockaddr *) &addr, connect_cb);

    // siridb_uv_connect(
    //     handle,
    //     loop,
    //     "iris",
    //     "siri",
    //     "dbtest",
    //     (const struct sockaddr *) &dest);

//     puts("Start siridb library test...");
//     int errn = 0;
//     siridb_t conn;
//     siridb_handle_t * handle = (siridb_handle_t *) malloc(sizeof(siridb_handle_t));

//     struct hostent *hp;
//     struct sockaddr_in sin;
//     memset(&sin, '\0', sizeof(sin));

//     int port = 9000;
// //    int rc;

//     hp = gethostbyname("localhost");
//     if ( hp == NULL ) {
//         fprintf(stderr, "host not found (%s)\n", "localhost");
//         exit(1);
//     }

//     sin.sin_family = AF_INET;
//     sin.sin_port = htons(port);
//     memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);

//     errn = siridb_init(
//             &conn,
//             &sin,
//             "iris", "siri", "dbtest");

//     uvwrap_init(&conn, loop);
//     siridb_handle_init(handle, &conn, on_data, NULL);
//     uvwrap_handle_init(handle);
//     siridb_connect(handle);

//     if (errn) {
//         puts(siridb_err_name(errn));
//         abort();
//     }


//     siridb_destroy(&conn);

    uv_run(loop, UV_RUN_DEFAULT);
    close_handlers();

    uv_loop_close(loop);
    free(loop);

// //    packer = qp_packer_create(QP_SUGGESTED_SIZE);
// //    qp_add_map(&packer);
// //    qp_add_raw(packer, "Hi Qpack", strlen("Hi Qpack"));
// //    qp_add_int64(packer, 9);
// //    qp_add_int64(packer, -79);
// //    qp_add_int64(packer, -1);
// //    qp_add_int64(packer, 123456789);
// //    qp_add_double(packer, 123.456);
// //    qp_close_map(packer);
// //    qp_packer_print(packer);
// //    qp_packer_destroy(packer);

    // printf("Finished siridb library test! (%d)\n", r);

    return 0;
}

// #endif /* TEST_ */
