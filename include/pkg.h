/*
 * pkg.h
 *
 *  Created on: Apr 07, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#ifndef SIRIDB_PKG_H_
#define SIRIDB_PKG_H_

#include <inttypes.h>
#include "series.h"

/* macros */
#define siridb_pkg_check_bit(PKG) (PKG->checkbit == (PKG->tp ^ 255))
#define siridb__pkg_set_checkbit(PKG) PKG->checkbit = PKG->tp ^ 255

/* type definitions */
typedef struct siridb_pkg_s siridb_pkg_t;

/* public functions */
#ifdef __cplusplus
extern "C" {
#endif

siridb_pkg_t * sirinet_pkg_create(
        uint16_t pid,
        uint8_t tp,
        const unsigned char * data,
        uint32_t len);
siridb_pkg_t * siridb_pkg_auth(
    uint16_t pid,
    const char * username,
    const char * password,
    const char * dbname);
siridb_pkg_t * siridb_pkg_query(uint16_t pid, const char * query);
siridb_pkg_t * siridb_pkg_series(
    uint16_t pid,
    siridb_series_t * series[],
    size_t n);
siridb_pkg_t * siridb_pkg_dup(siridb_pkg_t * pkg);

#ifdef __cplusplus
}
#endif

/* struct definitions */
struct siridb_pkg_s
{
    uint32_t len;
    uint16_t pid;
    uint8_t tp;
    uint8_t checkbit;
    unsigned char data[];
};

#endif /* SIRIDB_PKG_H_ */
