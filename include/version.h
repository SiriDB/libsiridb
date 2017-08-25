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
#define LIBSIRIDB_VERSION_PATCH 1

#define LIBSIRIDB_STRINGIFY(num) #num
#define LIBSIRIDB_VERSION_STR(major,minor,patch)    \
    LIBSIRIDB_STRINGIFY(major) "."                  \
    LIBSIRIDB_STRINGIFY(minor) "."                  \
    LIBSIRIDB_STRINGIFY(patch)

#define LIBSIRIDB_VERSION LIBSIRIDB_VERSION_STR(    \
        LIBSIRIDB_VERSION_MAJOR,                    \
        LIBSIRIDB_VERSION_MINOR,                    \
        LIBSIRIDB_VERSION_PATCH)

#ifdef __cplusplus
extern "C" {
#endif

const char * siridb_version(void);

#ifdef __cplusplus
}
#endif

#endif /* SIRIDB_VERSION_H_ */