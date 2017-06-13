/*
 * main.c
 *    SiriDB C-Connector example. Feel free to use/edit suv.c and suv.h in case
 *    you want to connect to SiriDB using libuv. This example is created so
 *    that the main file contains only real exampe code and the suv.c and suv.h
 *    file can be re-used for any libuv project.
 *
 *  WARNING:
 *     Be carefull runing this example since it will query and insert data into
 *     SiriDB!
 *
 *  Created on: Jun 09, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <uv.h>
#include <qpack.h>
#include <libsiridb/siridb.h>
#include "suv.h"

uv_loop_t loop;
uv_tcp_t tcp;

/* Change this values to your needs */
const char * SERVER = "127.0.0.1";
const int PORT = 9000;
const char * USER = "iris";
const char * PASSWD = "siri";
const char * DBNAME = "dbtest";
const char * QUERY = "select * from /.*/";

static void connect_cb(siridb_req_t * req);
static void query_cb(siridb_req_t * req);
static void insert_cb(siridb_req_t * req);
static void insert_example(siridb_t * siridb);
static void send_example_query(siridb_t * siridb, const char * query);
static void print_resp(siridb_resp_t * resp);
static void print_timeit(siridb_timeit_t * timeit);
static void print_select(siridb_select_t * select);
static void print_list(siridb_list_t * list);
static void print_count(uint64_t count);
static void print_calc(uint64_t calc);
static void print_show(siridb_show_t * show);
static void print_msg(const char * msg);
static void print_val(qp_res_t * val);

int main(void)
{
    struct sockaddr_in addr;
    char * query = strdup(QUERY);

    uv_loop_init(&loop);

    uv_ip4_addr(SERVER, PORT, &addr);

    siridb_t * siridb = siridb_create();
    /* handle siridb == NULL */

    suv_buf_t * buf = suv_buf_create(siridb);
    /* handle buf == NULL */

    siridb_req_t * req = siridb_req_create(siridb, connect_cb, NULL);
    /* handle req == NULL */

    suv_connect_t * connect = suv_connect_create(req, USER, PASSWD, DBNAME);
    /* handle connect == NULL */

    /* for the example we bind a query string but this could be anything */
    connect->data = (void *) query;
    req->data = (void *) connect;

    uv_tcp_init(&loop, &tcp);

    /* warn: this overwrites tcp->data so do not use the property yourself */
    suv_connect(connect, buf, &tcp, (struct sockaddr *) &addr);

    uv_run(&loop, UV_RUN_DEFAULT);
    uv_loop_close(&loop);

    /* cleanup buffer */
    suv_buf_destroy(buf);

    /* cleanup siridb */
    siridb_destroy(siridb);

    return 0;
}

static void connect_cb(siridb_req_t * req)
{
    /* handle req == NULL */
    suv_connect_t * connect = (suv_connect_t *) req->data;

    char * query = (char *) connect->data;
    if (req->status)
    {
        printf("connect failed: %s\n", siridb_strerror(req->status));
    }
    else
    {
        /* connect and authentication was succesful, send query */
        send_example_query(req->siridb, query);
    }

    /* free query string */
    free(query);

    /* destroy suv_connect_t */
    suv_connect_destroy(connect);

    /* destroy siridb request */
    siridb_req_destroy(req);
}

static void query_cb(siridb_req_t * req)
{
    siridb_t * siridb = req->siridb;

    if (req->status != 0)
    {
        printf("error handling request: %s", siridb_strerror(req->status));
    }
    else
    {
        siridb_resp_t * resp = siridb_resp_create(req, NULL);
        /* handle resp == NULL or us rc code for details */
        print_resp(resp);

        siridb_resp_destroy(resp);
    }

    /* destroy suv_query_t */
    suv_query_destroy((suv_query_t *) req->data);

    /* destroy siridb request */
    siridb_req_destroy(req);

    /* call the insert example */
    insert_example(siridb);
}

static void insert_cb(siridb_req_t * req)
{
    if (req->status != 0)
    {
        printf("error handling request: %s", siridb_strerror(req->status));
    }
    else
    {
        siridb_resp_t * resp = siridb_resp_create(req, NULL);
        /* handle resp == NULL or us rc code for details */
        print_resp(resp);

        siridb_resp_destroy(resp);
    }

    /* destroy suv_insert_t */
    suv_insert_destroy((suv_insert_t *) req->data);

    /* destroy siridb request */
    siridb_req_destroy(req);

    /* here we quit the example by closing the handle */
    if (!uv_is_closing((uv_handle_t *) &tcp))
    {
        uv_close((uv_handle_t *) &tcp, NULL);
    }
}

static void insert_example(siridb_t * siridb)
{
    siridb_series_t * series[2];
    siridb_point_t * point;

    series[0] = siridb_series_create(
        SIRIDB_SERIES_TP_INT64,         /* type integer */
        "c-conn-int64-test-series",     /* some name for the series */
        10);                            /* number of points */

    series[1] = siridb_series_create(
        SIRIDB_SERIES_TP_REAL,          /* type float */
        "c-conn-real-test-series",      /* some name for the series */
        5);                             /* number of points */

    for (size_t i = 0; i < series[0]->n; i++)
    {
        point = series[0]->points + i;
        /* set the time-stamp */
        point->ts = (uint64_t) time(NULL) - series[0]->n + i;
        /* set a value, just the values 0 to 9 in this example */
        point->via.int64 = (int64_t) i;
    }

    for (size_t i = 0; i < series[1]->n; i++)
    {
        point = series[1]->points + i;
        /* set the time-stamp */
        point->ts = (uint64_t) time(NULL) - series[1]->n + i;
        /* set a value, just the values 0.0 to 0.4 in this example */
        point->via.real = (double) i / 10;
    }

    siridb_req_t * req = siridb_req_create(siridb, insert_cb, NULL);
    /* handle req == NULL */

    suv_insert_t * suvinsert = suv_insert_create(req, series, 2);
    /* handle suvinsert == NULL */

    /* destroy the series, we don't need them anymore */
    siridb_series_destroy(series[0]);
    siridb_series_destroy(series[1]);

    /* bind suvinsert to qreq->data */
    req->data = (void *) suvinsert;

    suv_insert(suvinsert);
    /* check insert_cb for errors */
}

static void send_example_query(siridb_t * siridb, const char * query)
{
    siridb_req_t * req = siridb_req_create(siridb, query_cb, NULL);
    /* handle req == NULL */

    suv_query_t * suvquery = suv_query_create(req, query);
    /* handle suvquery == NULL */

    /* bind suvquery to qreq->data */
    req->data = (void *) suvquery;

    suv_query(suvquery);
    /* check query_cb for errors */
}

static void print_resp(siridb_resp_t * resp)
{
    print_timeit(resp->timeit);

    switch (resp->tp)
    {
    case SIRIDB_RESP_TP_SELECT:
        print_select(resp->via.select); break;
    case SIRIDB_RESP_TP_LIST:
        print_list(resp->via.list); break;
    case SIRIDB_RESP_TP_COUNT:
        print_count(resp->via.count); break;
    case SIRIDB_RESP_TP_CALC:
        print_calc(resp->via.calc); break;
    case SIRIDB_RESP_TP_SHOW:
        print_show(resp->via.show); break;
    case SIRIDB_RESP_TP_SUCCESS:
        print_msg(resp->via.success); break;
    case SIRIDB_RESP_TP_SUCCESS_MSG:
        print_msg(resp->via.success_msg); break;
    case SIRIDB_RESP_TP_ERROR:
        print_msg(resp->via.error); break;
    case SIRIDB_RESP_TP_ERROR_MSG:
        print_msg(resp->via.error_msg); break;
    default: assert(0);
    }
}

static void print_timeit(siridb_timeit_t * timeit)
{
    if (timeit != NULL)
    {
        printf("Query time: %f seconds\n", timeit->perfs[timeit->n - 1].time);
        for (size_t i = 0; i < timeit->n; i++)
        {
            printf(
                "    server: %s time: %f\n",
                timeit->perfs[i].server,
                timeit->perfs[i].time);
        }
        printf("\n");
    }
}

static void print_select(siridb_select_t * select)
{
    printf("Select response for %lu series:\n", select->n);
    for (size_t m = 0; m < select->n; m++)
    {
        siridb_series_t * series = select->series[m];

        printf("    series: '%s'\n", series->name);
        for (size_t i = 0; i < series->n; i++)
        {
            printf("        timestamp: %lu value: ", series->points[i].ts);
            switch (series->tp)
            {
            case SIRIDB_SERIES_TP_INT64:
                printf("%ld\n", series->points[i].via.int64); break;
            case SIRIDB_SERIES_TP_REAL:
                printf("%f\n", series->points[i].via.real); break;
            case SIRIDB_SERIES_TP_STR:
                printf("%s\n", series->points[i].via.str); break;
            }
        }
    }
}

static void print_list(siridb_list_t * list)
{
    printf(
        "List response with %lu columns and %lu rows:\n",
        list->headers->via.array->n,
        list->data->via.array->n);
    for (size_t r = 0; r < list->data->via.array->n; r++)
    {
        qp_array_t * row = list->data->via.array->values[r].via.array;
        for (size_t c = 0; c < row->n; c++)
        {
            if (c) printf(", ");
            print_val(row->values + c);
        }
        printf("\n");
    }
}

static void print_count(uint64_t count)
{
    printf("Count response: %lu\n", count);
}

static void print_calc(uint64_t calc)
{
    printf("Calc response: %lu\n", calc);
}

static void print_show(siridb_show_t * show)
{
    printf("Show response with %lu items\n", show->n);
    for (size_t i = 0; i < show->n; i++)
    {
        printf("    %s: ", show->items[i].key);
        print_val(show->items[i].value);
        printf("\n");
    }
}

static void print_msg(const char * msg)
{
    printf("%s\n", msg);
}

static void print_val(qp_res_t * val)
{
    switch (val->tp)
    {
    case QP_RES_MAP:    printf("{ ... }");              break;
    case QP_RES_ARRAY:  printf("[ ... ]");              break;
    case QP_RES_INT64:  printf("%ld", val->via.int64);  break;
    case QP_RES_REAL:   printf("%f", val->via.real);    break;
    case QP_RES_STR:    printf("%s", val->via.str);     break;
    case QP_RES_BOOL:   printf("%d", val->via.bool);    break;
    case QP_RES_NULL:   printf("null");                 break;
    }
}