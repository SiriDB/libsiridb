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
#include <uvwrap.h>
#include <qpack.h>

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
    siridb_handle_t * h1 = (siridb_handle_t *) malloc(sizeof(siridb_handle_t));
    siridb_handle_t * h2 = (siridb_handle_t *) malloc(sizeof(siridb_handle_t));

    siridb_handle_init(h1, handle->conn, on_data2, NULL);
    uvwrap_handle_init(h1);
    siridb_query(h1, "list serie name, start, end");

    siridb_handle_init(h2, handle->conn, on_data2, NULL);
    uvwrap_handle_init(h2);
    siridb_query(h2, "timeit select * from /.*/");
    siridb_handle_destroy(handle);
    free(handle);
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

int main(void)
{
//    qp_packer_t * packer;
    uv_idle_t idler;
    loop = (uv_loop_t *) malloc(sizeof(uv_loop_t));
    uv_loop_init(loop);

    uv_idle_init(loop, &idler);
    uv_idle_start(&idler, wait_for_a_while);

    puts("Start siridb library test...");
    int errn = 0;
    siridb_t conn;
    siridb_handle_t * handle = (siridb_handle_t *) malloc(sizeof(siridb_handle_t));

    struct hostent *hp;
    struct sockaddr_in sin;
    memset(&sin, '\0', sizeof(sin));

    int port = 9000;
//    int rc;

    hp = gethostbyname("localhost");
    if ( hp == NULL ) {
        fprintf(stderr, "host not found (%s)\n", "localhost");
        exit(1);
    }

    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);

    errn = siridb_init(
            &conn,
            &sin,
            "iris", "siri", "dbtest");

    uvwrap_init(&conn, loop);
    siridb_handle_init(handle, &conn, on_data, NULL);
    uvwrap_handle_init(handle);
    siridb_connect(handle);

    if (errn) {
        puts(siridb_err_name(errn));
        abort();
    }

    uv_run(loop, UV_RUN_DEFAULT);

    close_handlers();

    siridb_destroy(&conn);


    int r = uv_loop_close(loop);
    free(loop);

//    packer = qp_packer_create(QP_SUGGESTED_SIZE);
//    qp_add_map(&packer);
//    qp_add_raw(packer, "Hi Qpack", strlen("Hi Qpack"));
//    qp_add_int64(packer, 9);
//    qp_add_int64(packer, -79);
//    qp_add_int64(packer, -1);
//    qp_add_int64(packer, 123456789);
//    qp_add_double(packer, 123.456);
//    qp_close_map(packer);
//    qp_packer_print(packer);
//    qp_packer_destroy(packer);

    printf("Finished siridb library test! (%d)\n", r);

    return 0;
}

// #endif /* TEST_ */
