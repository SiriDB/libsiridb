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
#include "point.h"

/* typedefs */
typedef struct siridb_series_s siridb_series_t;

/* enums */
typedef enum siridb_series_e
{
    SIRIDB_SERIES_TP_INT64,
    SIRIDB_SERIES_TP_REAL,
    SIRIDB_SERIES_TP_STR
} siridb_series_tp;

/* public functions */
#ifdef __cplusplus
extern "C" {
#endif

siridb_series_t * siridb_series_create(
        siridb_series_tp tp,
        char * name,
        size_t size);
void siridb_series_destroy(siridb_series_t * series);
int siridb_series_resize(siridb_series_t ** series, size_t n);

#ifdef __cplusplus
}
#endif

/* private */
siridb_series_tp siridb__series_get_tp(qp_array_t * points);

/* struct definitions */
struct siridb_series_s
{
    siridb_series_tp tp;
    char * name;
    size_t n;
    siridb_point_t points[];
};

#endif /* SIRIDB_SERIES_H_ */
