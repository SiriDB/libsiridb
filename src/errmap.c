/*
 * errmap.c
 *
 *  Created on: Apr 07, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#include <errmap.h>
#include <siridb.h>
#include <errno.h>

const char * siridb_strerror(int err)
{
    switch (err)
    {
    case 0:                 return "success";
    case ERR_MEM_ALLOC:     return "memory allocation failed";
    case ERR_SOCK_FD:       return "cannot create socket file descriptor";
    case ERR_SOCK_CONNECT:  return "cannot open socket";
    case ERR_SOCK_WRITE:    return "cannot write to socket";
    case ERR_PENDING:       return "request is pending";
    case ERR_CANCELLED:     return "request is cancelled";
    case ERR_INVALID_STAT:  return "request has an invalid status";
    case ERR_OVERWRITTEN:   return "request is overwritten before handled";
    case ERR_NOT_FOUND:     return "request not found, most likely it was cancelled";
    case ERR_CORRUPT:       return "data is corrupt";
    case ERR_OCCUPIED:      return "already occupied";
    case ERR_UNKNOWN:
    default:                return "unknown";
    }
}
