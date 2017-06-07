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
#include <siridbuv.h>

int results = 0;
uv_loop_t * loop;

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

static void on_data2(siridb_handle_t * handle)
{
    printf("On Data 2...%d (pid: %u)\n", handle->status, handle->pkg->pid);
    if (handle->status == 0)
    {
        siridb_resp_t resp;
        int r = siridb_resp_init(&resp, handle);
        printf("Resp parse status: %d\n", r);
        if (resp.timeit != NULL)
        {
            printf("got timeit for server: %s with time: %f\n",
                    resp.timeit->perfs[0].server,
                    resp.timeit->perfs[0].time);
        }
        if (resp.tp == SIRIDB_RESP_TP_SELECT)
        {
            siridb_select_t * select = resp.via.select;
            printf("Select response for %lu series.\n", select->n);
            for (size_t m = 0; m < select->n; m++)
            {
                printf("Series: %s (points: %lu, type: %d)\n",
                        select->series[m]->name,
                        select->series[m]->n,
                        select->series[m]->tp);
            }
        }
        if (resp.tp == SIRIDB_RESP_TP_LIST)
        {
            siridb_list_t * list = resp.via.list;
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
        if (resp.tp == SIRIDB_RESP_TP_COUNT)
        {
            printf("Count response: %lu\n", resp.via.count);
        }
        if (resp.tp == SIRIDB_RESP_TP_CALC)
        {
            printf("Calc response: %lu\n", resp.via.calc);
        }
        if (resp.tp == SIRIDB_RESP_TP_SHOW)
        {
            size_t i;
            siridb_show_t * show = resp.via.show;
            printf("Show response with %lu items\n", show->n);
            for (i = 0; i < show->n; i++)
            {
                printf("name: %s    value tp: ", show->items[i].key);
                print_val(show->items[i].value);
                printf("\n");
            }
        }
        if (resp.tp == SIRIDB_RESP_TP_ERROR_MSG)
        {
            printf("Error msg: %s\n", resp.via.error_msg);
        }
        if (resp.tp == SIRIDB_RESP_TP_SUCCESS_MSG)
        {
            printf("Success msg: %s\n", resp.via.success_msg);
        }
        if (resp.tp == SIRIDB_RESP_TP_ERROR)
        {
            printf("Error: %s\n", resp.via.error);
        }
        if (resp.tp == SIRIDB_RESP_TP_SUCCESS)
        {
            printf("Success: %s\n", resp.via.success);
        }
        siridb_resp_destroy(&resp);
    }
    siridb_handle_destroy(handle);
    free(handle);
    results++;

}

static void on_data(siridb_handle_t * handle)
{
    printf("Here...%d\n", handle->status);

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

static void auth_cb(siridb_req_t req)
{
    printf("On Auth Cb");
}

const char * SERVER = "127.0.0.1";
const int PORT = 9000;
const char * USER = "iris";
const char * PASSWD = "siri";
const char * DBNAME = "dbtest";
const int SUGGESTED_SIZE = 65536;

static void write_cb(uv_write_t * uvreq, int status)
{
    if (status)
    {
        /* error handling */
        printf("cannot write to socket: %s\n",  uv_strerror(status));
    }
    /* free siridb_pkg_t */
    free(uvreq->data);

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

        siridb_t * siridb = (siridb_t *) uvreq->data;
        siridb_req_t * req = siridb_req_create(siridb, auth_cb);
        /* check for not NULL */

        siridb_pkg_t * pkg = siridb_pkg_auth(req->pid, USER, PASSWD, DBNAME);
        /* check for not NULL */

        uv_write_t * uvreq = (uv_write_t *) malloc(sizeof(uv_write_t));
        /* check for not NULL */

        /* we bind pkg to data so we can free pkg when write is done */
        uvreq->data = pkg;

        uv_buf_t buf = uv_buf_init(
                (char *) pkg,
                sizeof(siridb_pkg_t) + pkg->len);


        uv_write(req, (uv_stream_t *) uvreq->handle, &buf, 1, write_cb);
    }
}

typedef struct store_s store_t;

struct store_s
{
    char * buf;
    size_t len;
    size_t size;
}

static void alloc_buffer(uv_handle_t * handle, size_t sug_size, uv_buf_t * buf)
{
    store_t * store = (store_t *) handle->data;

    if (store->buf == NULL)
    {
        /* check for not NULL */
        store->buf = (char *) malloc(sug_size);
        store->size = sug_size;
        store->len = 0;
    }
    buf->base = store->buf + store->len;
    buf->len = store->size - store->len;
}

void on_data(uv_stream_t * client, ssize_t nread, const uv_buf_t * buf)
{
    store_t * store = (store_t *) client->data;

    if (nread < 0)
    {
        if (nread != UV_EOF)
        {
            printf("read error: %s\n", uv_err_name(nread));
            /* error handling */
        }
        uv_close((uv_handle_t *) client, NULL);
        return;
    }

    store->len += nread;

    if (store->len > )

}

int main(void)
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
    /* check for not NULL */

    uv_connect_t * uvreq = (uv_connect_t *) malloc(sizeof(uv_connect_t));
    /* check for not NULL */

    uvreq->data = (void *) siridb;

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

    int r = uv_loop_close(loop);
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
