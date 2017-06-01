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
int siridb_connect(siridb_handle_t * handle);

void siridb_handle_init(
        siridb_handle_t * handle,
        siridb_t * conn,
        siridb_cb cb,
        void * arg);
void siridb_handle_destroy(siridb_handle_t * handle);

int siridb_resp_init(siridb_resp_t * resp, siridb_handle_t * handle);
void siridb_resp_destroy(siridb_resp_t * resp);

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
    siridb_thread_t connthread;
    imap_t * imap;
    char * buf;
    size_t buf_sz;
    size_t buf_len;
    uint16_t pid;
};

#endif /* SIRIDB_H_ */
