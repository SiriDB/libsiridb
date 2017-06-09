/*
 * suv.h - SiriDB C-Connector, example using libuv
 *
 *  Created on: Jun 09, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#ifndef SUV_H_
#define SUV_H_

#include <siridb/siridb.h>
#include <uv.h>

typedef struct suv_buf_s suv_buf_t;

suv_buf_t * suv_buf_create(siridb_t * siridb);
void suv_buf_destroy(suv_buf_t * suvbf);
void suv_connect_cb(uv_connect_t * uvreq, int status);
suv_auth_t * suv_auth_create(const char * usr, * pwd, * dbname, siridb_cb cb);
void suv_auth_destroy(suv_auth_t * auth);

struct suv_buf_s
{
    char * buf;
    size_t len;
    size_t size;
    siridb_t * siridb;
};

struct suv_auth_s
{
    char * username;
    char * password;
    char * dbname;
    siridb_cb cb;
}

#endif /* SUV_H_ */