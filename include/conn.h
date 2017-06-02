/*
 * conn.h
 *
 *  Created on: Jun 02, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#ifndef SIRIDB_CONN_H_
#define SIRIDB_CONN_H_

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

#endif /* SIRIDB_CONN_H_ */