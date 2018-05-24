/*
 * version.h
 *
 *  Created on: Jun 14, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */
#ifndef SIRIDB_VERSION_H_
#define SIRIDB_VERSION_H_

#define LIBSIRIDB_VERSION_MAJOR 0
#define LIBSIRIDB_VERSION_MINOR 9
#define LIBSIRIDB_VERSION_PATCH 3

#define LIBSIRIDB_VERSION "0.9.3"

#ifdef __cplusplus
extern "C" {
#endif

const char * siridb_version(void);

#ifdef __cplusplus
}
#endif

#endif /* SIRIDB_VERSION_H_ */
