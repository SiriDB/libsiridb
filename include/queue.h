/*
 * queue.h
 *
 *  Created on: Apr 07, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#ifndef SIRIDB_QUEUE__H_
#define SIRIDB_QUEUE__H_

#include <inttypes.h>
#include <stddef.h>

/* typedefs */
typedef struct queue_node_s queue_node_t;
typedef struct queue_s queue_t;
typedef void (*queue_cb)(void * data);

/* private functions */
queue_t * queue_create(void);
void queue_destroy(queue_t * queue);
int queue_add(queue_t * queue, uint64_t id, void * data);
void * queue_get(queue_t * queue, uint64_t id);
void * queue_pop(queue_t * queue, uint64_t id);
void queue_walk(queue_t * queue, queue_cb cb);

/* struct definitions */
struct queue_node_s
{
    size_t size;
    void * data ;
    queue_node_t * nodes;
};

struct queue_s
{
    size_t len;
    queue_node_t nodes[];
};

#endif /* SIRIDB_QUEUE__H_ */
