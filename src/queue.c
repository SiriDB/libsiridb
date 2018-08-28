/*
 * queue.c
 *
 *  Created on: Apr 07, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#include <queue.h>
#include <assert.h>
#include <stdlib.h>
#include <errmap.h>

#define QUEUE_NODE_SZ 32

static void queue__node_free(queue_node_t * node);
static int queue__add(queue_node_t * node, uint64_t id, void * data);
static void * queue__get(queue_node_t * node, uint64_t id);
static void * queue__pop(queue_node_t * node, uint64_t id);
static void queue__walk(queue_node_t * node, queue_cb cb);


/*
 * Returns NULL in case an allocation error has occurred.
 */
queue_t * queue_create(void)
{
    queue_t * queue = (queue_t *) calloc(
            1,
            sizeof(queue_t) + QUEUE_NODE_SZ * sizeof(queue_node_t));

    if (queue != NULL)
    {
        queue->len = 0;
    }
    return queue;
}

/*
 * Destroy queue with optional call-back function.
 */
void queue_destroy(queue_t * queue)
{
    if (queue->len)
    {
        queue_node_t * nd;

        for (uint8_t i = 0; i < QUEUE_NODE_SZ; i++)
        {
            nd = queue->nodes + i;

            if (nd->nodes != NULL)
            {
                queue__node_free(nd);
            }
        }
    }

    free(queue);
}

/*
 * Add data by id to the map.
 *
 * Returns 0 when successful of a negative value in case of an error.
 *
 * possible error codes:
 *   ERR_OCCUPIED   : when the id already exists
 *   ERR_MEM_ALLOC  : memory allocation error has occurred
 */
int queue_add(queue_t * queue, uint64_t id, void * data)
{
    /* insert NULL is not allowed */
    assert (data != NULL);

    int rc = 0;
    queue_node_t * nd = queue->nodes + (id % QUEUE_NODE_SZ);
    id /= QUEUE_NODE_SZ;

    if (!id)
    {
        if (nd->data != NULL)
        {
            return ERR_OCCUPIED;
        }

        queue->len++;
        nd->data = data;
    }
    else if ((rc = queue__add(nd, id - 1, data)) == 0)
    {
        queue->len++;
    }

    return rc;
}

/*
 * Returns data by a given id, or NULL when not found.
 */
void * queue_get(queue_t * queue, uint64_t id)
{
    queue_node_t * nd = queue->nodes + (id % QUEUE_NODE_SZ);
    id /= QUEUE_NODE_SZ;

    if (!id)
    {
        return nd->data;
    }

    return (nd->nodes == NULL) ? NULL : queue__get(nd, id - 1);
}

/*
 * Remove and return an item by id or return NULL in case the id is not found.
 */
void * queue_pop(queue_t * queue, uint64_t id)
{
    void * data;
    queue_node_t * nd = queue->nodes + (id % QUEUE_NODE_SZ);
    id /= QUEUE_NODE_SZ;

    if (id)
    {
        data = (nd->nodes == NULL) ? NULL : queue__pop(nd, id - 1);
    }
    else if ((data = nd->data) != NULL)
    {
        nd->data = NULL;
    }

    if (data != NULL)
    {
        queue->len--;
    }

    return data;
}

/*
 * Run the call-back function on all items in the map.
 *
 * All the results are added together and are returned as the result of
 * this function.
 */
void queue_walk(queue_t * queue, queue_cb cb)
{
    if (queue->len)
    {
        queue_node_t * nd;

        for (uint8_t i = 0; i < QUEUE_NODE_SZ; i++)
        {
            nd = queue->nodes + i;

            if (nd->data != NULL)
            {
                (*cb)(nd->data);
            }

            if (nd->nodes != NULL)
            {
                queue__walk(nd, cb);
            }
        }
    }
}

static void queue__node_free(queue_node_t * node)
{
    queue_node_t * nd;

    for (uint8_t i = 0; i < QUEUE_NODE_SZ; i++)
    {
        if ((nd = node->nodes + i)->nodes != NULL)
        {
            queue__node_free(nd);
        }
    }

    free(node->nodes);
}

/*
 * Add data by id to the map.
 *
 * Returns 0 when successful or a negative value in case of an error.
 */
static int queue__add(queue_node_t * node, uint64_t id, void * data)
{
    if (!node->size)
    {
        node->nodes = (queue_node_t *) calloc(
                QUEUE_NODE_SZ,
                sizeof(queue_node_t));

        if (node->nodes == NULL)
        {
            return ERR_MEM_ALLOC;
        }
    }

    int rc;
    queue_node_t * nd = node->nodes + (id % QUEUE_NODE_SZ);
    id /= QUEUE_NODE_SZ;

    if (!id)
    {
        if (nd->data != NULL)
        {
            return ERR_OCCUPIED;
        }

        nd->data = data;
        node->size++;

        return 0;
    }

    if ((rc = queue__add(nd, id - 1, data)) == 0)
    {
        node->size++;
    }

    return rc;
}

static void * queue__get(queue_node_t * node, uint64_t id)
{
    queue_node_t * nd = node->nodes + (id % QUEUE_NODE_SZ);
    id /= QUEUE_NODE_SZ;

    if (!id)
    {
        return nd->data;
    }

    return (nd->nodes == NULL) ? NULL : queue__get(nd, id - 1);
}

static void * queue__pop(queue_node_t * node, uint64_t id)
{
    void * data;
    queue_node_t * nd = node->nodes + (id % QUEUE_NODE_SZ);
    id /= QUEUE_NODE_SZ;

    if (!id)
    {
        if ((data = nd->data) != NULL)
        {
            if (--node->size)
            {
                nd->data = NULL;
            }
            else
            {
                free(node->nodes);
                node->nodes = NULL;
            }
        }

        return data;
    }

    data = (nd->nodes == NULL) ? NULL : queue__pop(nd, id - 1);

    if (data != NULL && !--node->size)
    {
        free(node->nodes);
        node->nodes = NULL;
    }

    return data;
}

static void queue__walk(queue_node_t * node, queue_cb cb)
{
    queue_node_t * nd;

    for (uint8_t i = 0; i < QUEUE_NODE_SZ; i++)
    {
        nd = node->nodes + i;

        if (nd->data != NULL)
        {
            (*cb)(nd->data);
        }

        if (nd->nodes != NULL)
        {
            queue__walk(nd, cb);
        }
    }
}

