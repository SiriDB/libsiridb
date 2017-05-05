/*
 * errmap.c
 *
 *  Created on: Apr 07, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#include <errmap.h>
#include <siridb.h>
#include <errno.h>

const char * siridb_err_name(int err)
{
    switch (err)
    {
    case ERR_MEM_ALLOC:     return "memory allocation failed";
    case ERR_SOCK_FD:       return "cannot create socket file descriptor";
    case ERR_SOCK_CONNECT:  return "cannot open socket";
    case ERR_UNFINISHED:    return "handle is not finished";
    case ERR_THREAD_START:  return "cannot start thread";
    case ERR_NO_REPLY:      return "overwritten before reply";
    case ERR_CANCELLED:     return "handle is cancelled";
    case ERR_INVALID_STAT:  return "invalid handle status";
    case ERR_CORRUPT:       return "data is corrupt";
    case ERR_DESTROYED:     return "handle is destroyed";
    case ERR_UNKNOWN:
    default:                return "unknown";
    }
}
