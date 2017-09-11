/*
 * series.c
 *
 *  Created on: May 04, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#include <siridb.h>
#include <stdlib.h>
#include <string.h>

/*
 * Create a series. Argument name is allowed to be NULL. In this case
 * series->name will be set to NULL and must be set manually using heap memory
 * allocation so that the name can be freed by siridb_series_destroy().
 */
siridb_series_t * siridb_series_create(
        siridb_series_tp tp,
        const char * name,
        size_t n)
{
    siridb_series_t * series = (siridb_series_t *) malloc(
            sizeof(siridb_series_t) + n * sizeof(siridb_point_t));

    if (series == NULL)
    {
        return NULL;
    }

    series->tp = tp;

    if (name == NULL)
    {
        series->name = NULL;
    }
    else if ((series->name = strdup(name)) == NULL)
    {
        free(series);
        return NULL;
    }

    series->n = n;

    if (series->tp == SIRIDB_SERIES_TP_STR)
    {
        /* initialize string series with NULL pointers */
        size_t i;
        for (i = 0; i < n; i++)
        {
            series->points[i].via.str = NULL;
        }
    }

    return series;
}

void siridb_series_destroy(siridb_series_t * series)
{
    if (series->tp == SIRIDB_SERIES_TP_STR)
    {
        size_t i;
        for (i = 0; i < series->n; i++)
        {
            free(series->points[i].via.str);
        }
    }
    free(series->name);
    free(series);
}

int siridb_series_resize(siridb_series_t ** series, size_t n)
{
    siridb_series_t * tmp;

    tmp = (siridb_series_t *) realloc(
            *series,
            sizeof(siridb_series_t) + n * sizeof(siridb_point_t));

    if (tmp == NULL)
    {
        /* an error has occurred */
        return ERR_MEM_ALLOC;
    }

    /* overwrite the original value with the new one */
    tmp->n = n;
    *series = tmp;

    return 0;
}

siridb_series_tp siridb__series_get_tp(qp_array_t * points)
{
    qp_array_t * point;

    if (!points->n)
    {
        /* return integer type in case the series has no points */
        return SIRIDB_SERIES_TP_INT64;
    }

    if (points->values[0].tp != QP_RES_ARRAY ||
        (point = points->values[0].via.array)->n != 2)
    {
        return -1;
    }

    switch (point->values[1].tp)
    {
    case QP_RES_INT64:
        return SIRIDB_SERIES_TP_INT64;
    case QP_RES_REAL:
        return SIRIDB_SERIES_TP_REAL;
    case QP_RES_STR:
        return SIRIDB_SERIES_TP_STR;
    case QP_RES_MAP:
    case QP_RES_ARRAY:
    case QP_RES_BOOL:
    case QP_RES_NULL:
    default:
        return -1;
    }
}
