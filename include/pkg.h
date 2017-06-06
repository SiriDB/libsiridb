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
#define SIRIDB_PKG_MAX_SZ 524288000 // 500MB

typedef struct siridb_pkg_s siridb_pkg_t;

struct siridb_pkg_s
{
    uint32_t len;
    uint16_t pid;
    uint8_t tp;
    uint8_t checkbit;
    unsigned char data[];
};

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

siridb_pkg_t * siridb_pkg_query(
    const char * query,
    uint16_t timeout);

siridb_pkg_t * siridb_pkg_dup(siridb_pkg_t * pkg);

#define siridb_pkg_set_checkbit(PKG) PKG->checkbit = PKG->tp ^ 255
#define siridb_pkg_check_bit(PKG) PKG->checkbit == (PKG->tp ^ 255)

#endif /* SIRIDB_PKG_H_ */
