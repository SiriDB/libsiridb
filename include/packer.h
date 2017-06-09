/*
 * packer.h
 *
 *  Created on: Jun 02, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#ifndef SIRIDB_PACKER_H_
#define SIRIDB_PACKER_H_

#include <inttypes.h>
#include <qpack.h>
#include "pkg.h"

/* type definitions */
typedef qp_packer_t siridb_packer_t;    /* alias */

/* public functions */
siridb_packer_t * siridb_packer_create(size_t alloc_size);
void siridb_packer_destroy(siridb_packer_t * packer);
siridb_pkg_t * siridb_packer_2pkg(
    siridb_packer_t * packer,
    uint16_t pid,
    uint8_t tp);

#endif /* SIRIDB_PACKER_H_ */