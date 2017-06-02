/*
 * imap.h
 *
 *  Created on: Apr 07, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#ifndef SIRIDB_IMAP_H_
#define SIRIDB_IMAP_H_

#include <inttypes.h>
#include <stddef.h>

typedef struct imap_node_s imap_node_t;
typedef struct imap_s imap_t;

typedef int (*imap_cb)(void * data, void * args);
typedef void (*imap_free_cb)(void * data);

struct imap_node_s
{
    size_t size;
    void * data ;
    imap_node_t * nodes;
};

struct imap_s
{
    size_t len;
    imap_node_t nodes[];
};

imap_t * imap_create(void);
void imap_destroy(imap_t * imap, imap_free_cb cb);
int imap_add(imap_t * imap, uint64_t id, void * data);
void * imap_get(imap_t * imap, uint64_t id);
void * imap_pop(imap_t * imap, uint64_t id);
int imap_walk(imap_t * imap, imap_cb cb, void * data);

#endif /* SIRIDB_IMAP_H_ */
