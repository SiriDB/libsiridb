/*
 * errmap.h
 *
 *  Created on: Apr 07, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#ifndef SIRIDB_ERRMAP_H_
#define SIRIDB_ERRMAP_H_

#define ERR_MEM_ALLOC       -1
#define ERR_SOCK_FD         -2
#define ERR_SOCK_CONNECT    -3
#define ERR_SOCK_WRITE      -4
#define ERR_PENDING         -5
#define ERR_CANCELLED       -6
#define ERR_INVALID_STAT    -7
#define ERR_OVERWRITTEN     -8
#define ERR_NOT_FOUND       -9
#define ERR_CORRUPT         -10
#define ERR_OCCUPIED        -11
#define ERR_UNKNOWN         -100

const char * siridb_strerror(int err);

#endif /* SIRIDB_ERRMAP_H_ */
