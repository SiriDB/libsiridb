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

int results = 0;

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
        int r = siridb_resp_read(&resp, handle);
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
                printf("Series: %s (points: %lu)\n",
                        select->series[m]->name,
                        select->series[m]->n);
            }
        }
        if (resp.tp == SIRIDB_RESP_TP_LIST)
        {
            siridb_list_t * list = resp.via.list;
            printf("List response with %lu columns and %lu rows...\n",
                    list->headers->n, list->data->n);
            for (size_t r = 0; r < list->data->n; r++)
            {
                qp_array_t * row = list->data->values[r].via.array;
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
    }
    results++;

}

static void on_data(siridb_handle_t * handle)
{
    printf("Here...%d\n", handle->status);
    siridb_handle_t * h1 = (siridb_handle_t *) malloc(sizeof(siridb_handle_t));
    siridb_handle_t * h2 = (siridb_handle_t *) malloc(sizeof(siridb_handle_t));

    siridb_handle_init(h1, handle->conn, on_data2, NULL);
    uvwrap_handle_init(h1);
    siridb_query(h1, "1d");

    siridb_handle_init(h2, handle->conn, on_data2, NULL);
    uvwrap_handle_init(h2);
    siridb_query(h2, "timeit select * from /.*/");
}

int main(void)
{
//    qp_packer_t * packer;
    uv_idle_t idler;

    uv_idle_init(uv_default_loop(), &idler);
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

    uvwrap_init(&conn, uv_default_loop());
    siridb_handle_init(handle, &conn, on_data, NULL);
    uvwrap_handle_init(handle);
    siridb_connect(handle);

    if (errn) {
        puts(siridb_err_name(errn));
        abort();
    }

    uv_run(uv_default_loop(), UV_RUN_DEFAULT);

    siridb_destroy(&conn);

    uv_loop_close(uv_default_loop());



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

    puts("Finished siridb library test!");

    return 0;
}

// #endif /* TEST_ */
