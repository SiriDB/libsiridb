/*
 * conn.h
 *
 *  Created on: Jun 02, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#ifndef SIRIDB_CONN_H_
#define SIRIDB_CONN_H_

#include <inttypes.h>
#include <imap.h>

typedef struct siridb_conn_s siridb_conn_t;

struct siridb_conn_s
{
    void * data;  /* public */
    uint16_t pid;
    imap_t * imap;
};

siridb_conn_t * siridb_conn_create(void * data);
void siridb_conn_destroy(siridb_conn_t * conn);

#endif /* SIRIDB_CONN_H_ */