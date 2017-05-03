/*
 * thread.c
 *
 *  Created on: Apr 07, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#include <resp.h>
#include <stddef.h>
#include <errmap.h>
#include <protomap.h>

static int siridb_resp_query(siridb_resp_t * resp, siridb_pkg_t * pkg);
static int siridb_resp_timeit(siridb_resp_t * resp, qp_map_t * map);
static int siridb_resp_select(siridb_resp_t * resp, qp_map_t * map);
static int siridb_resp_list(siridb_resp_t * resp, qp_map_t * map);
static int siridb_resp_show(siridb_resp_t * resp, qp_map_t * map);
static int siridb_resp_count(siridb_resp_t * resp, qp_map_t * map);
static int siridb_resp_calc(siridb_resp_t * resp, qp_map_t * map);
static int siridb_resp_success(siridb_resp_t * resp, qp_map_t * map);
static int siridb_resp_error(siridb_resp_t * resp, qp_map_t * map);
static int siridb_resp_help(siridb_resp_t * resp, qp_map_t * map);
static int siridb_resp_motd(siridb_resp_t * resp, qp_map_t * map);

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

int siridb_resp_read(siridb_resp_t * resp, siridb_handle_t * handle)
{
    resp->tp = SIRIDB_RESP_TP_UNDEF;
    resp->timeit = NULL;

    if (handle->status != 0)
    {
        return ERR_INVALID_STATUS;
    }

    assert (handle->pkg != NULL);

    switch(handle->pkg->tp)
    {
    case CprotoResQuery:
        return siridb_resp_query(resp, handle->pkg);
    }
    return 0;
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
    case QP_ERR_CORRUPT:
        return ERR_CORRUPT;

    case QP_ERR_ALLOC:
        return ERR_MEM_ALLOC;

    case 0:
        if (res->tp != QP_RES_MAP)
        {
            rc = ERR_CORRUPT;
            break;
        }

        if ((rc = siridb_resp_timeit(resp, res->via.map)))
        {
            break;
        }

        if ((rc = siridb_resp_list(resp, res->via.map)) || resp->tp)
        {
            break;
        }

        if ((rc = siridb_resp_count(resp, res->via.map)) || resp->tp)
        {
            break;
        }

        if ((rc = siridb_resp_show(resp, res->via.map)) || resp->tp)
        {
            break;
        }

        if ((rc = siridb_resp_calc(resp, res->via.map)) || resp->tp)
        {
            break;
        }

        if ((rc = siridb_resp_success(resp, res->via.map)) || resp->tp)
        {
            break;
        }

        if ((rc = siridb_resp_help(resp, res->via.map)) || resp->tp)
        {
            break;
        }

        if ((rc = siridb_resp_motd(resp, res->via.map)) || resp->tp)
        {
            break;
        }

        if ((rc = siridb_resp_select(resp, res->via.map)) || resp->tp)
        {
            break;
        }

        break;
    default:
        rc = ERR_UNKNOWN;
    }

    qp_res_destroy(res);

    if (!resp->tp)
    {
        rc = ERR_CORRUPT;
    }

    return rc;
}

static int siridb_resp_timeit(siridb_resp_t * resp, qp_map_t * map)
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

                            if (perf->server == NULL)
                            {
                                return ERR_MEM_ALLOC;
                            }

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
    /* no timeit information found */
    return 0;
}

static int siridb_resp_select(siridb_resp_t * resp, qp_map_t * map)
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
            siridb_series_tp tp = siridb_points_get_tp(points);
            qp_array_t * point;

            if (tp < 0)
            {
                return ERR_CORRUPT;
            }

            series = siridb_series_create(tp, &map->keys[i].via.str, points->n);

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

static int siridb_resp_list(siridb_resp_t * resp, qp_map_t * map)
{
    qp_res_t * tmp;
    tmp = siridb_map_get(map, "columns");
    if (tmp != NULL &&
        tmp->tp == QP_RES_ARRAY &&
        tmp->via.array->n &&
        tmp->via.array->values[0].tp == QP_RES_STR)
    {
        int i;

        resp->via.list = (siridb_list_t *) malloc(sizeof(siridb_list_t));
        if (resp->via.list == NULL)
        {
            return ERR_MEM_ALLOC;
        }
        resp->tp = SIRIDB_RESP_TP_LIST;

        /* move header */
        resp->via.list->headers = tmp->via.array;
        tmp->tp = QP_RES_NULL;

        for (i = 0; i < list_num; i++)
        {
            tmp = siridb_map_get(map, list_types[i]);
            if (tmp != NULL && tmp->tp == QP_RES_ARRAY)
            {
                /* move data */
                resp->via.list->data = tmp->via.array;
                tmp->tp = QP_RES_NULL;
                return 0;
            }
        }

        resp->via.list->data = NULL;
        return ERR_CORRUPT;
    }

    /* not a list result, return successful */
    return 0;
}

static int siridb_resp_show(siridb_resp_t * resp, qp_map_t * map)
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
                sizeof(siridb_item_t) * tmp->via.array->n);

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
                item = &show->items[i];

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

static int siridb_resp_count(siridb_resp_t * resp, qp_map_t * map)
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

static int siridb_resp_calc(siridb_resp_t * resp, qp_map_t * map)
{
    qp_res_t * tmp = siridb_map_get(map, "calc");

    if (tmp != NULL && tmp->tp == QP_RES_INT64)
    {
        resp->tp = SIRIDB_RESP_TP_CALC;
        resp->via.calc = (uint64_t) tmp->via.int64;
    }
    return 0;
}

static int siridb_resp_success(siridb_resp_t * resp, qp_map_t * map)
{
    qp_res_t * tmp = siridb_map_get(map, "success_msg");

    if (tmp != NULL && tmp->tp == QP_RES_STR)
    {
        resp->tp = SIRIDB_RESP_TP_SUCCESS;
        resp->via.success = tmp->via.str;
        tmp->via.str = NULL;
    }
    return 0;
}

static int siridb_resp_error(siridb_resp_t * resp, qp_map_t * map)
{
    qp_res_t * tmp = siridb_map_get(map, "error_msg");

    if (tmp != NULL && tmp->tp == QP_RES_STR)
    {
        resp->tp = SIRIDB_RESP_TP_ERROR;
        resp->via.error = tmp->via.str;
        tmp->via.str = NULL;
    }
    return 0;
}

static int siridb_resp_help(siridb_resp_t * resp, qp_map_t * map)
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

static int siridb_resp_motd(siridb_resp_t * resp, qp_map_t * map)
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






