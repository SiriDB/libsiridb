/*
 * siridbuv.h
 *
 *  Created on: Jun 02, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */
#ifdef USE_UV
#ifndef SIRIDB_SIRIDBUV_H_
#define SIRIDB_SIRIDBUV_H_

typedef struct siridb_conn_s siridb_conn_t;

struct siridb_conn_s
{
    uint16_t pid;
    char * username;
    char * password;
    char * dbname;
    imap_t * imap;
    void * _conn;  /* depending on implementation */
};

#endif /* SIRIDB_SIRIDBUV_H_ */
#endif /* USE_UV */