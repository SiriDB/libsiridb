/*
 * thread.c
 *
 *  Created on: Apr 07, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#include <siridb.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

static int siridb_resp_success(siridb_resp_t * resp, uint8_t tp);
static int siridb_resp_error(siridb_resp_t * resp, uint8_t tp);
static int siridb_resp_query(siridb_resp_t * resp, siridb_pkg_t * pkg);
static int siridb_resp_success_msg(siridb_resp_t * resp, siridb_pkg_t * pkg);
static int siridb_resp_error_msg(siridb_resp_t * resp, siridb_pkg_t * pkg);
static int siridb_resp_data(siridb_resp_t * resp, siridb_pkg_t * pkg);
static int siridb_resp_get_timeit(siridb_resp_t * resp, qp_map_t * map);
static int siridb_resp_get_select(siridb_resp_t * resp, qp_map_t * map);
static int siridb_resp_get_list(siridb_resp_t * resp, qp_map_t * map);
static int siridb_resp_get_show(siridb_resp_t * resp, qp_map_t * map);
static int siridb_resp_get_count(siridb_resp_t * resp, qp_map_t * map);
static int siridb_resp_get_calc(siridb_resp_t * resp, qp_map_t * map);
static int siridb_resp_get_success(siridb_resp_t * resp, qp_map_t * map);
static int siridb_resp_get_help(siridb_resp_t * resp, qp_map_t * map);
static int siridb_resp_get_motd(siridb_resp_t * resp, qp_map_t * map);
static qp_res_t * siridb_map_get(qp_map_t * map, const char * key);

static const int list_num = 3;
static const char * list_types[] = {
        "series",
        "pools",
        "servers"
};

static const int count_num = 9;
static const char * count_types[] = {
        "series",
        "servers",
        "groups",
        "shards",
        "pools",
        "users",
        "servers_received_points",
        "series_length",
        "shards_size"
};

/*
 * Initialize siridb_resp_t object.
 * Returns 0 if successful or a negative value in case of an error.
 *
 */
siridb_resp_t * siridb_resp_create(siridb_pkg_t * pkg, int * rc)
{
    assert (pkg != NULL);

    int fallb;
    int * rcode = (rc == NULL) ? &fallb : rc;
    siridb_resp_t * resp;

    resp = (siridb_resp_t *) malloc(sizeof(siridb_resp_t));
    if (resp == NULL)
    {
        *rcode = ERR_MEM_ALLOC;
        return NULL;
    }

    resp->tp = SIRIDB_RESP_TP_UNDEF;
    resp->timeit = NULL;

    switch(pkg->tp)
    {
    case CprotoResQuery:
        *rcode = siridb_resp_query(resp, pkg);
        break;
    case CprotoResInfo:
    case CprotoAckAdminData:
        *rcode = siridb_resp_data(resp, pkg);
        break;
    case CprotoResInsert:
        *rcode = siridb_resp_success_msg(resp, pkg);
        break;
    case CprotoResAuthSuccess:
    case CprotoResAck:
    case CprotoAckAdmin:
        *rcode = siridb_resp_success(resp, pkg->tp);
        break;
    case CprotoErrMsg:
    case CprotoErrUserAccess:
    case CprotoErrPool:
    case CprotoErrServer:
    case CprotoErrQuery:
    case CprotoErrInsert:
    case CprotoErrAdmin:
        *rcode = siridb_resp_error_msg(resp, pkg);
        break;
    default:
        *rcode = siridb_resp_error(resp, pkg->tp);
    }

    if (*rcode)
    {
        siridb_resp_destroy(resp);
        resp = NULL;
    }

    return resp;
}

void siridb_resp_destroy(siridb_resp_t * resp)
{
    size_t n;
    if (resp->timeit != NULL)
    {
        for (n = 0; n < resp->timeit->n; n++)
        {
            free(resp->timeit->perfs[n].server);
        }
        free(resp->timeit);
    }
    switch (resp->tp)
    {
    case SIRIDB_RESP_TP_UNDEF:
    case SIRIDB_RESP_TP_COUNT:
    case SIRIDB_RESP_TP_CALC:
    case SIRIDB_RESP_TP_SUCCESS:
    case SIRIDB_RESP_TP_ERROR:
        break;
    case SIRIDB_RESP_TP_SELECT:
        for (n = 0; n < resp->via.select->n; n++)
        {
            siridb_series_destroy(resp->via.select->series[n]);
        }
        free(resp->via.select);
        break;
    case SIRIDB_RESP_TP_DATA:
        qp_res_destroy(resp->via.data);
        break;
    case SIRIDB_RESP_TP_LIST:
        qp_res_destroy(resp->via.list->headers);
        qp_res_destroy(resp->via.list->data);
        free(resp->via.list);
        break;
    case SIRIDB_RESP_TP_SHOW:
        for (n = 0; n < resp->via.show->n; n++)
        {
            free(resp->via.show->items[n].key);
            qp_res_destroy(resp->via.show->items[n].value);
        }
        free(resp->via.show);
        break;
    case SIRIDB_RESP_TP_SUCCESS_MSG:
        free(resp->via.success_msg);
        break;
    case SIRIDB_RESP_TP_ERROR_MSG:
        free(resp->via.error_msg);
        break;
    case SIRIDB_RESP_TP_HELP:
        free(resp->via.help);
        break;
    case SIRIDB_RESP_TP_MOTD:
        free(resp->via.motd);
        break;
    default:
        assert(0);
    }
    free(resp);
}

static int siridb_resp_success(siridb_resp_t * resp, uint8_t tp)
{
    resp->tp = SIRIDB_RESP_TP_SUCCESS;
    switch (tp)
    {
    case CprotoResAuthSuccess:
        resp->via.success = "authentication successful";
        break;
    case CprotoResAck:
        resp->via.success = "ack received";
        break;
    case CprotoAckAdmin:
        resp->via.success = "service ack received";
        break;
    default:
        assert (0);
    }

    return 0;
}

static int siridb_resp_error(siridb_resp_t * resp, uint8_t tp)
{
    resp->tp = SIRIDB_RESP_TP_ERROR;

    switch (tp)
    {
    case CprotoErrAdminInvalidRequest:
        resp->via.error = "invalid request";
        break;
    case CprotoErr:
        resp->via.error = "runtime error";
        break;
    case CprotoErrNotAuthenticated:
        resp->via.error = "not authenticated";
        break;
    case CprotoErrAuthCredentials:
        resp->via.error = "invalid credentials";
        break;
    case CprotoErrAuthUnknownDb:
        resp->via.error = "unknown database";
        break;
    case CprotoErrLoadingDb:
        resp->via.error = "error loading database";
        break;
    case CprotoErrFile:
        resp->via.error = "error while downloading file";
        break;
    default:
        resp->via.error = "unknown package type";
        break;
    }

    return 0;
}

static int siridb_resp_success_msg(siridb_resp_t * resp, siridb_pkg_t * pkg)
{
    int rc;
    qp_unpacker_t unpacker;
    qp_res_t * res, * msg;

    qp_unpacker_init(&unpacker, pkg->data, pkg->len);
    res = qp_unpacker_res(&unpacker, &rc);

    switch(rc)
    {
    case 0:
        if (res->tp == QP_RES_MAP &&
            (msg = siridb_map_get(res->via.map, "success_msg")) != NULL &&
            msg->tp == QP_RES_STR)
        {
            resp->tp = SIRIDB_RESP_TP_SUCCESS_MSG;
            resp->via.success_msg = msg->via.str;
            msg->tp = QP_RES_NULL;
        }
        else
        {
            rc = ERR_CORRUPT;
        }
        break;
    case QP_ERR_CORRUPT:    return ERR_CORRUPT;
    case QP_ERR_ALLOC:      return ERR_MEM_ALLOC;
    default:                return ERR_UNKNOWN;
    }

    qp_res_destroy(res);
    return rc;
}

static int siridb_resp_error_msg(siridb_resp_t * resp, siridb_pkg_t * pkg)
{
    int rc;
    qp_unpacker_t unpacker;
    qp_res_t * res, * msg;

    qp_unpacker_init(&unpacker, pkg->data, pkg->len);
    res = qp_unpacker_res(&unpacker, &rc);

    switch(rc)
    {
    case 0:
        if (res->tp == QP_RES_MAP &&
            (msg = siridb_map_get(res->via.map, "error_msg")) != NULL &&
            msg->tp == QP_RES_STR)
        {
            resp->tp = SIRIDB_RESP_TP_ERROR_MSG;
            resp->via.error_msg = msg->via.str;
            msg->tp = QP_RES_NULL;
            rc = 0;
        }
        else
        {
            rc = ERR_CORRUPT;
        }
        break;
    case QP_ERR_CORRUPT:    return ERR_CORRUPT;
    case QP_ERR_ALLOC:      return ERR_MEM_ALLOC;
    default:                return ERR_UNKNOWN;
    }

    qp_res_destroy(res);
    return rc;
}

static int siridb_resp_data(siridb_resp_t * resp, siridb_pkg_t * pkg)
{
    int rc;
    qp_unpacker_t unpacker;

    qp_unpacker_init(&unpacker, pkg->data, pkg->len);
    resp->via.data = qp_unpacker_res(&unpacker, &rc);
    switch(rc)
    {
    case 0:
        resp->tp = SIRIDB_RESP_TP_DATA;
        return 0;
    case QP_ERR_CORRUPT:    return ERR_CORRUPT;
    case QP_ERR_ALLOC:      return ERR_MEM_ALLOC;
    default:                return ERR_UNKNOWN;
    }
}

static int siridb_resp_query(siridb_resp_t * resp, siridb_pkg_t * pkg)
{
    int rc;
    qp_unpacker_t unpacker;
    qp_res_t * res;

    qp_unpacker_init(&unpacker, pkg->data, pkg->len);
    res = qp_unpacker_res(&unpacker, &rc);

    switch(rc)
    {
    case 0:
        if (res->tp != QP_RES_MAP)
        {
            rc = ERR_CORRUPT;
            break;
        }

        if ((rc = siridb_resp_get_timeit(resp, res->via.map)))
        {
            break;
        }

        if ((rc = siridb_resp_get_list(resp, res->via.map)) || resp->tp)
        {
            break;
        }

        if ((rc = siridb_resp_get_count(resp, res->via.map)) || resp->tp)
        {
            break;
        }

        if ((rc = siridb_resp_get_show(resp, res->via.map)) || resp->tp)
        {
            break;
        }

        if ((rc = siridb_resp_get_calc(resp, res->via.map)) || resp->tp)
        {
            break;
        }

        if ((rc = siridb_resp_get_success(resp, res->via.map)) || resp->tp)
        {
            break;
        }

        if ((rc = siridb_resp_get_help(resp, res->via.map)) || resp->tp)
        {
            break;
        }

        if ((rc = siridb_resp_get_motd(resp, res->via.map)) || resp->tp)
        {
            break;
        }

        if ((rc = siridb_resp_get_select(resp, res->via.map)) || resp->tp)
        {
            break;
        }

        rc = ERR_CORRUPT;
        break;
    case QP_ERR_CORRUPT:    return ERR_CORRUPT;
    case QP_ERR_ALLOC:      return ERR_MEM_ALLOC;
    default:                return ERR_UNKNOWN;
    }

    qp_res_destroy(res);
    return rc;
}

static int siridb_resp_get_timeit(siridb_resp_t * resp, qp_map_t * map)
{
    int i;
    for (i = 0; i < map->n; i++)
    {
        if (map->keys[i].tp == QP_RES_STR &&
            strcmp(map->keys[i].via.str, "__timeit__") == 0 &&
            map->values[i].tp == QP_RES_ARRAY)
        {
            int j;
            qp_array_t * timeits = map->values[i].via.array;
            resp->timeit = (siridb_timeit_t *) malloc(
                    sizeof(siridb_timeit_t) +
                    sizeof(siridb_perf_t) * timeits->n);
            if (resp->timeit == NULL)
            {
                return ERR_MEM_ALLOC;
            }
            resp->timeit->n = timeits->n;
            for (j = 0; j < timeits->n; j++)
            {
                siridb_perf_t * perf = &resp->timeit->perfs[j];
                if (timeits->values[j].tp == QP_RES_MAP &&
                    timeits->values[j].via.map->n == 2)
                {
                    int n = 2;
                    qp_map_t * tm = timeits->values[j].via.map;
                    while (n--)
                    {
                        if (tm->keys[n].tp == QP_RES_STR &&
                            strcmp(tm->keys[n].via.str, "time") == 0 &&
                            tm->values[n].tp == QP_RES_REAL)
                        {
                            perf->time = tm->values[n].via.real;
                            continue;
                        }

                        if (tm->keys[n].tp == QP_RES_STR &&
                            strcmp(tm->keys[n].via.str, "server") == 0 &&
                            tm->values[n].tp == QP_RES_STR)
                        {
                            perf->server = tm->values[n].via.str;
                            tm->values[n].via.str= NULL;

                            continue;
                        }
                        return ERR_CORRUPT;
                    }
                    continue;
                }
                return ERR_CORRUPT;
            }
            return 0;
        }
    }
    /* no time-it information found */
    return 0;
}

static int siridb_resp_get_select(siridb_resp_t * resp, qp_map_t * map)
{
    size_t i, n;
    siridb_series_t * series;

    resp->via.select = (siridb_select_t *) malloc(
            sizeof(siridb_select_t) +
            sizeof(siridb_series_t *) * (map->n - (resp->timeit != NULL)));

    if (resp->via.select == NULL)
    {
        return ERR_MEM_ALLOC;
    }

    resp->tp = SIRIDB_RESP_TP_SELECT;
    resp->via.select->n = 0;

    for (i = 0; i < map->n; i++)
    {
        if (map->keys[i].tp == QP_RES_STR &&
            strncmp(map->keys[i].via.str, "__", 2) &&
            map->values[i].tp == QP_RES_ARRAY)
        {
            qp_array_t * points = map->values[i].via.array;
            siridb_series_tp tp = siridb__series_get_tp(points);
            qp_array_t * point;

            if (tp < 0)
            {
                return ERR_CORRUPT;
            }

            series = siridb_series_create(tp, NULL, points->n);

            /* move the name rather than copy to avoid allocation */
            series->name = map->keys[i].via.str;
            map->keys[i].via.str = NULL;

            if (series == NULL)
            {
                return ERR_MEM_ALLOC;
            }

            for (n = 0; n < points->n; n++)
            {
                if (points->values[n].tp != QP_RES_ARRAY ||
                    (point = points->values[n].via.array)->n != 2)
                {
                    siridb_series_destroy(series);
                    return ERR_CORRUPT;
                }

                series->points[n].ts = (uint64_t) point->values[0].via.int64;

                switch (tp)
                {
                case SIRIDB_SERIES_TP_INT64:
                    series->points[n].via.int64 = point->values[1].via.int64;
                    break;
                case SIRIDB_SERIES_TP_REAL:
                    series->points[n].via.real = point->values[1].via.real;
                    break;
                case SIRIDB_SERIES_TP_STR:
                    /* move the point so we do not have to allocate memory */
                    series->points[n].via.str = point->values[1].via.str;
                    point->values[1].via.str = NULL;
                    break;
                default:
                    /* this cannot happen */
                    assert(0);
                }
            }

            resp->via.select->series[resp->via.select->n++] = series;
        }
    }
    return 0;
}

static int siridb_resp_get_list(siridb_resp_t * resp, qp_map_t * map)
{
    qp_res_t * qcols, * qrows = NULL;

    qcols = siridb_map_get(map, "columns");

    if (qcols != NULL &&
        qcols->tp == QP_RES_ARRAY &&
        qcols->via.array->n &&
        qcols->via.array->values[0].tp == QP_RES_STR)
    {
        int i;
        for (i = 0; i < list_num; i++)
        {
            qrows = siridb_map_get(map, list_types[i]);
            if (qrows != NULL && qrows->tp == QP_RES_ARRAY)
            {
                break;
            }
        }
    }

    if (qrows != NULL)
    {
        siridb_list_t * list = resp->via.list =
                (siridb_list_t *) malloc(sizeof(siridb_list_t));
        if (list == NULL)
        {
            return ERR_MEM_ALLOC;
        }

        list->headers = (qp_res_t *) malloc(sizeof(qp_res_t));
        list->data = (qp_res_t *) malloc(sizeof(qp_res_t));

        if (list->headers == NULL || list->data == NULL)
        {
            free(list->headers);
            free(list->data);
            free(list);
            return ERR_MEM_ALLOC;
        }

        resp->tp = SIRIDB_RESP_TP_LIST;

        list->headers->tp = qcols->tp;
        list->data->tp = qrows->tp;

        memcpy(
            &list->headers->via,
            &qcols->via,
            sizeof(qp_res_via_t));

        memcpy(
            &list->data->via,
            &qrows->via,
            sizeof(qp_res_via_t));

        qcols->tp = QP_RES_NULL;
        qrows->tp = QP_RES_NULL;
    }

    /* not a list result, return successful */
    return 0;
}

static int siridb_resp_get_show(siridb_resp_t * resp, qp_map_t * map)
{
    qp_array_t * arr;
    qp_res_t * tmp;

    if ((tmp = siridb_map_get(map, "data")) != NULL &&
        tmp->tp == QP_RES_ARRAY &&
        (arr = tmp->via.array)->n &&
        (tmp = &arr->values[0])->tp == QP_RES_MAP &&
        siridb_map_get(tmp->via.map, "name") != NULL)
    {
        size_t i;
        qp_res_t * name, * value;
        siridb_item_t * item;
        siridb_show_t * show;

        resp->via.show = show = (siridb_show_t *) malloc(
                sizeof(siridb_show_t) +
                sizeof(siridb_item_t) * arr->n);

        if (show == NULL)
        {
            return ERR_MEM_ALLOC;
        }

        resp->tp = SIRIDB_RESP_TP_SHOW;
        show->n = 0;

        for (i = 0; i < arr->n; i++)
        {
            tmp = arr->values + i;
            if (tmp->tp == QP_RES_MAP &&
                (name = siridb_map_get(tmp->via.map, "name")) != NULL &&
                name->tp == QP_RES_STR &&
                (value = siridb_map_get(tmp->via.map, "value")) != NULL)
            {
                item = &show->items[show->n];

                item->key = name->via.str;
                item->value = (qp_res_t *) malloc(sizeof(qp_res_t));

                if (item->value == NULL)
                {
                    return ERR_MEM_ALLOC;
                }

                item->value->tp = value->tp;

                memcpy(
                    &item->value->via,
                    &value->via,
                    sizeof(qp_res_via_t));

                value->tp = QP_RES_NULL;
                name->tp = QP_RES_NULL;
                show->n++;
            }
            else
            {
                return ERR_CORRUPT;
            }
        }
    }
    return 0;
}

static int siridb_resp_get_count(siridb_resp_t * resp, qp_map_t * map)
{
    size_t i;
    qp_res_t * tmp;

    for (i = 0; i < count_num; i++)
    {
        tmp = siridb_map_get(map, count_types[i]);

        if (tmp != NULL && tmp->tp == QP_RES_INT64)
        {
            resp->tp = SIRIDB_RESP_TP_COUNT;
            resp->via.count = (uint64_t) tmp->via.int64;
            return 0;
        }
    }

    return 0;
}

static int siridb_resp_get_calc(siridb_resp_t * resp, qp_map_t * map)
{
    qp_res_t * tmp = siridb_map_get(map, "calc");

    if (tmp != NULL && tmp->tp == QP_RES_INT64)
    {
        resp->tp = SIRIDB_RESP_TP_CALC;
        resp->via.calc = (uint64_t) tmp->via.int64;
    }
    return 0;
}

static int siridb_resp_get_success(siridb_resp_t * resp, qp_map_t * map)
{
    qp_res_t * tmp = siridb_map_get(map, "success_msg");

    if (tmp != NULL && tmp->tp == QP_RES_STR)
    {
        resp->tp = SIRIDB_RESP_TP_SUCCESS_MSG;
        resp->via.success_msg = tmp->via.str;
        tmp->via.str = NULL;
    }
    return 0;
}

static int siridb_resp_get_help(siridb_resp_t * resp, qp_map_t * map)
{
    qp_res_t * tmp = siridb_map_get(map, "help");

    if (tmp != NULL && tmp->tp == QP_RES_STR)
    {
        resp->tp = SIRIDB_RESP_TP_HELP;
        resp->via.help = tmp->via.str;
        tmp->via.str = NULL;
    }
    return 0;
}

static int siridb_resp_get_motd(siridb_resp_t * resp, qp_map_t * map)
{
    qp_res_t * tmp = siridb_map_get(map, "motd");

    if (tmp != NULL && tmp->tp == QP_RES_STR)
    {
        resp->tp = SIRIDB_RESP_TP_MOTD;
        resp->via.motd = tmp->via.str;
        tmp->via.str = NULL;
    }
    return 0;
}

static qp_res_t * siridb_map_get(qp_map_t * map, const char * key)
{
    size_t i;

    for (i = 0; i < map->n; i++)
    {
        if (map->keys[i].tp == QP_RES_STR &&
            strcmp(map->keys[i].via.str, key) == 0)
        {
            return map->values + i;
        }
    }

    return NULL;
}

