/*
 * pkg.h
 *
 *  Created on: Apr 07, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#ifndef SIRIDB_PKG_H_
#define SIRIDB_PKG_H_

#include <inttypes.h>

#define SIRIDB_PKG_SZ 8
#define SIRIDB_PKG_MAX_SZ 52428800

typedef struct siridb_pkg_s siridb_pkg_t;

struct siridb_pkg_s
{
    uint32_t len;
    uint16_t pid;
    uint8_t tp;
    uint8_t checkbit;
    unsigned char data[];
};

siridb_pkg_t * sirinet_pkg_new(
        uint8_t tp,
        uint16_t pid,
        uint32_t len,
        const unsigned char * data);

siridb_pkg_t * siridb_pkg_dup(siridb_pkg_t * pkg);

#endif /* SIRIDB_PKG_H_ */
