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
#include <resp.h>
#include <errmap.h>
#include <protomap.h>
#include <series.h>
#include <handle.h>
#include <netinet/in.h>


typedef struct siridb_s siridb_t;

struct siridb_s
{
    void * data;        /* public */
    uint16_t pid;
    imap_t * imap;
};

siridb_t * siridb_create(void);
void siridb_destroy(siridb_t * siridb);


typedef void (*siridb_cb) (siridb_handle_t * handle);

void siridb_query(siridb_handle_t * handle, const char * query);
void siridb_send(siridb_handle_t * handle, siridb_pkg_t * pkg);

const char * siridb_err_name(int err);


int siridb_resp_init(siridb_resp_t * resp, siridb_handle_t * handle);
void siridb_resp_destroy(siridb_resp_t * resp);

void siridb__on_pkg(siridb_conn_t * conn, siridb_pkg_t * pkg);
void siridb__write(siridb_handle_t * handle, siridb_pkg_t * pkg);


#endif /* SIRIDB_H_ */
