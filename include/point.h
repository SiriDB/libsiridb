/*
 * point.h
 *
 *  Created on: Apr 07, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#ifndef SIRIDB_POINT_H_
#define SIRIDB_POINT_H_

typedef union siridb_point_u siridb_point_via_t;
typedef struct siridb_point_s siridb_point_t;

union siridb_point_u
{
    int64_t int64;
    double real;
    char * str;     /* null terminated string */
};

struct siridb_point_s
{
    uint64_t ts;                /* time-stamp */
    siridb_point_via_t via;     /* value, type is set in series */
};

#endif /* SIRIDB_POINT_H_ */
