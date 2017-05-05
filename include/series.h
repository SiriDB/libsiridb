/*
 * series.h
 *
 *  Created on: May 04, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#ifndef SIRIDB_SERIES_H_
#define SIRIDB_SERIES_H_

#include <qpack.h>
#include <stddef.h>
#include <point.h>

typedef enum siridb_series_e siridb_series_tp;
typedef struct siridb_series_s siridb_series_t;

/* public */
siridb_series_t * siridb_series_create(
        siridb_series_tp tp,
        char * name,
        size_t n);
void siridb_series_destroy(siridb_series_t * series);

/* private */
siridb_series_tp siridb__series_get_tp(qp_array_t * points);

enum siridb_series_e
{
    SIRIDB_SERIES_TP_INT64,
    SIRIDB_SERIES_TP_REAL,
    SIRIDB_SERIES_TP_STR
};

struct siridb_series_s
{
    siridb_series_tp tp;
    char * name;
    size_t n;
    siridb_point_t points[];
};

#endif /* SIRIDB_SERIES_H_ */
