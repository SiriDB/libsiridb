/*
 * siridb.h
 *
 *  Created on: Apr 07, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#ifndef SIRIDB_H_
#define SIRIDB_H_

#include <inttypes.h>
#include <stddef.h>
#include <thread.h>
#include <imap.h>
#include <pkg.h>
#include <qpack.h>

typedef union siridb_point_u siridb_point_via_t;
typedef struct siridb_point_s siridb_point_t;

typedef enum siridb_series_e siridb_series_tp;
typedef struct siridb_series_s siridb_series_t;


typedef struct siridb_s siridb_t;
typedef struct siridb_handle_s siridb_handle_t;

/*
 * status: Status is zero when successful or a another value when something
 *         went wrong. (for example when a query error occurs).
 */
typedef void (*siridb_cb) (siridb_handle_t * handle);

void siridb_query(siridb_handle_t * handle, const char * query);
const char * siridb_err_name(int err);
int siridb_init(
        siridb_t * conn,
        struct sockaddr_in * addr,
        const char * user,
        const char * password,
        const char * dbname);
void siridb_destroy(siridb_t * conn);
void siridb_handle_init(
        siridb_handle_t * handle,
        siridb_t * conn,
        siridb_cb cb,
        void * arg);
int siridb_connect(siridb_handle_t * handle);

enum siridb_series_e
{
    SIRIDB_SERIES_TP_INT64,
    SIRIDB_SERIES_TP_REAL,
    SIRIDB_SERIES_TP_STR
};

union siridb_point_u
{
    int64_t int64;
    double real;
    char * str;     /* null terminated string */
};

struct siridb_s
{
    void * data;
    siridb_cb cb;

    char * username;
    char * password;
    char * dbname;
    struct sockaddr_in * addr;
    int sockfd;
    siridb_thread_t thread;
    imap_t * imap;
    siridb_mutex_t mutex;
    char * buf;
    size_t buf_sz;
    size_t buf_len;
    uint16_t pid;
};

struct siridb_handle_s
{
    siridb_t * conn;
    siridb_cb cb;
    void * arg;
    int status;
    siridb_pkg_t * pkg;
};

struct siridb_point_s
{
    uint64_t ts;                /* time-stamp */
    siridb_point_via_t via;     /* value */
};

struct siridb_series_s
{
    siridb_series_tp tp;
    char * name;
    size_t n;
    siridb_point_t points[];
};




#endif /* SIRIDB_H_ */
