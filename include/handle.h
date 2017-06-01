/*
 * handle.h
 *
 *  Created on: May 04, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#ifndef SIRIDB_HANDLE_H_
#define SIRIDB_HANDLE_H_

typedef struct siridb_handle_s siridb_handle_t;
typedef struct siridb_s siridb_t;
typedef void (*siridb_cb) (siridb_handle_t * handle);

void siridb_handle_cancel(siridb_t * conn, siridb_handle_t * handle);
void siridb__handle_cancel(siridb_handle_t * handle);

struct siridb_handle_s
{
    uint16_t pid;
    uint16_t _pad0;
    int status;
    siridb_cb cb;
    siridb_t * conn;
    void * arg;
    siridb_pkg_t * pkg;
};

#endif /* SIRIDB_HANDLE_H_ */
