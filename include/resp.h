/*
 * resp.h
 *
 *  Created on: Apr 07, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#ifndef SIRIDB_RESP_H_
#define SIRIDB_RESP_H_

#include <qpack.h>
#include <series.h>

/* response definitions */
typedef enum siridb_resp_e siridb_resp_tp;
typedef union siridb_resp_u siridb_resp_via_t;
typedef struct siridb_resp_s siridb_resp_t;

/* result and helper definitions */
typedef struct siridb_list_s siridb_list_t;
typedef struct siridb_select_s siridb_select_t;
typedef struct siridb_show_s siridb_show_t;
typedef struct siridb_timeit_s siridb_timeit_t;
typedef struct siridb_item_s siridb_item_t;
typedef struct siridb_perf_s siridb_perf_t;

enum siridb_resp_e
{
    SIRIDB_RESP_TP_UNDEF=0,
    SIRIDB_RESP_TP_SELECT,
    SIRIDB_RESP_TP_LIST,
    SIRIDB_RESP_TP_SHOW,
    SIRIDB_RESP_TP_COUNT,
    SIRIDB_RESP_TP_CALC,
    SIRIDB_RESP_TP_SUCCESS,
    SIRIDB_RESP_TP_SUCCESS_MSG,
    SIRIDB_RESP_TP_ERROR,
    SIRIDB_RESP_TP_ERROR_MSG,
    SIRIDB_RESP_TP_HELP,
    SIRIDB_RESP_TP_DATA,
    SIRIDB_RESP_TP_MOTD     /* only when SiriDB server is in debug mode */
};

union siridb_resp_u {
    siridb_select_t * select;
    siridb_list_t * list;
    siridb_show_t * show;
    qp_res_t * data;
    uint64_t count;
    uint64_t calc;
    char * success;
    char * error;
    char * success_msg;
    char * error_msg;
    char * help;
    char * motd;  /* only when SiriDB server is in debug mode */
};

struct siridb_resp_s
{
    siridb_resp_tp tp;
    siridb_resp_via_t via;
    siridb_timeit_t * timeit;
};

struct siridb_perf_s
{
    char * server;
    double time;
};

struct siridb_item_s
{
    char * key;
    qp_res_t * value;
};

struct siridb_list_s
{
    qp_res_t * headers;
    qp_res_t * data;
};

struct siridb_select_s
{
    size_t n;
    siridb_series_t * series[];
};

struct siridb_show_s
{
    size_t n;
    siridb_item_t items[];
};

struct siridb_timeit_s
{
    size_t n;
    siridb_perf_t perfs[];
};

#endif /* SIRIDB_RESP_H_ */
