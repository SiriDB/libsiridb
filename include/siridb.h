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


typedef void (*siridb_cb) (siridb_handle_t * handle);

void siridb_query(siridb_handle_t * handle, const char * query);
void siridb_send(siridb_handle_t * handle, siridb_pkg_t * pkg);

const char * siridb_err_name(int err);

siridb_handle_t * siridb_handle_create(
        siridb_conn_t * conn,
        siridb_cb cb,
        void * arg);
void siridb_handle_destroy(siridb_handle_t * handle);
void siridb_handle_cancel(siridb_conn_t * conn, siridb_handle_t * handle);

int siridb_resp_init(siridb_resp_t * resp, siridb_handle_t * handle);
void siridb_resp_destroy(siridb_resp_t * resp);

void siridb__on_pkg(siridb_conn_t * conn, siridb_pkg_t * pkg);
int siridb__write(siridb_handle_t * handle, siridb_pkg_t * pkg);


#endif /* SIRIDB_H_ */
