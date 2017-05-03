/*
 * imap.c
 *
 *  Created on: Apr 07, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#include <imap.h>
#include <assert.h>
#include <stdlib.h>

#define IMAP_NODE_SZ 32

static void IMAP_node_free(imap_node_t * node);
static void IMAP_node_free_cb(imap_node_t * node, imap_free_cb cb);
static int IMAP_add(imap_node_t * node, uint64_t id, void * data);
static void * IMAP_get(imap_node_t * node, uint64_t id);
static void * IMAP_pop(imap_node_t * node, uint64_t id);
static void IMAP_walk(imap_node_t * node, imap_cb cb, void * data, int * rc);


/*
 * Returns NULL in case an allocation error has occurred.
 */
imap_t * imap_new(void)
{
    imap_t * imap = (imap_t *) calloc(
            1,
            sizeof(imap_t) + IMAP_NODE_SZ * sizeof(imap_node_t));

    if (imap != NULL)
    {
        imap->len = 0;
    }
    return imap;
}

/*
 * Destroy imap with optional call-back function.
 */
void imap_free(imap_t * imap, imap_free_cb cb)
{
    if (imap->len)
    {
        imap_node_t * nd;

        if (cb == NULL)
        {
            for (uint8_t i = 0; i < IMAP_NODE_SZ; i++)
            {
                nd = imap->nodes + i;

                if (nd->nodes != NULL)
                {
                    IMAP_node_free(nd);
                }
            }
        }
        else
        {
            for (uint8_t i = 0; i < IMAP_NODE_SZ; i++)
            {
                nd = imap->nodes + i;

                if (nd->data != NULL)
                {
                    (*cb)(nd->data);
                }

                if (nd->nodes != NULL)
                {
                    IMAP_node_free_cb(nd, cb);
                }
            }
        }
    }

    free(imap);
}

/*
 * Add data by id to the map.
 *
 * Returns 0 when data is overwritten and 1 if a new id/value is set.
 *
 * In case of an error we return -1 and a SIGNAL is raised.
 */
int imap_add(imap_t * imap, uint64_t id, void * data)
{
#ifdef DEBUG
    /* insert NULL is not allowed */
    assert (data != NULL);
#endif
    int rc;
    imap_node_t * nd = imap->nodes + (id % IMAP_NODE_SZ);
    id /= IMAP_NODE_SZ;

    if (!id)
    {
        rc = (nd->data == NULL);

        imap->len += rc;
        nd->data = data;
    }
    else
    {
        rc = IMAP_add(nd, id - 1, data);

        if (rc > 0)
        {
            imap->len++;
        }
    }


    return rc;
}

/*
 * Returns data by a given id, or NULL when not found.
 */
void * imap_get(imap_t * imap, uint64_t id)
{
    imap_node_t * nd = imap->nodes + (id % IMAP_NODE_SZ);
    id /= IMAP_NODE_SZ;

    if (!id)
    {
        return nd->data;
    }

    return (nd->nodes == NULL) ? NULL : IMAP_get(nd, id - 1);
}

/*
 * Remove and return an item by id or return NULL in case the id is not found.
 */
void * imap_pop(imap_t * imap, uint64_t id)
{
    void * data;
    imap_node_t * nd = imap->nodes + (id % IMAP_NODE_SZ);
    id /= IMAP_NODE_SZ;

    if (id)
    {
        data = (nd->nodes == NULL) ? NULL : IMAP_pop(nd, id - 1);
    }
    else if ((data = nd->data) != NULL)
    {
        nd->data = NULL;
    }

    if (data != NULL)
    {
        imap->len--;
    }

    return data;
}

/*
 * Run the call-back function on all items in the map.
 *
 * All the results are added together and are returned as the result of
 * this function.
 */
int imap_walk(imap_t * imap, imap_cb cb, void * data)
{
    int rc = 0;

    if (imap->len)
    {
        imap_node_t * nd;

        for (uint8_t i = 0; i < IMAP_NODE_SZ; i++)
        {
            nd = imap->nodes + i;

            if (nd->data != NULL)
            {
                rc += (*cb)(nd->data, data);
            }

            if (nd->nodes != NULL)
            {
                IMAP_walk(nd, cb, data, &rc);
            }
        }
    }

    return rc;
}

static void IMAP_node_free(imap_node_t * node)
{
    imap_node_t * nd;

    for (uint8_t i = 0; i < IMAP_NODE_SZ; i++)
    {
        if ((nd = node->nodes + i)->nodes != NULL)
        {
            IMAP_node_free(nd);
        }
    }

    free(node->nodes);
}

static void IMAP_node_free_cb(imap_node_t * node, imap_free_cb cb)
{
    imap_node_t * nd;

    for (uint8_t i = 0; i < IMAP_NODE_SZ; i++)
    {
        nd = node->nodes + i;

        if (nd->data != NULL)
        {
            (*cb)(nd->data);
        }

        if (nd->nodes != NULL)
        {
            IMAP_node_free_cb(nd, cb);
        }
    }
    free(node->nodes);
}

/*
 * Add data by id to the map.
 *
 * Returns 0 when data is overwritten and 1 if a new id/value is set.
 *
 * In case of an error -1 will be returned.
 */
static int IMAP_add(imap_node_t * node, uint64_t id, void * data)
{
    if (!node->size)
    {
        node->nodes = (imap_node_t *) calloc(
                IMAP_NODE_SZ,
                sizeof(imap_node_t));

        if (node->nodes == NULL)
        {
            return -1;
        }
    }

    int rc;
    imap_node_t * nd = node->nodes + (id % IMAP_NODE_SZ);
    id /= IMAP_NODE_SZ;

    if (!id)
    {
        rc = (nd->data == NULL);

        nd->data = data;
        node->size += rc;

        return rc;
    }

    rc = IMAP_add(nd, id - 1, data);

    if (rc > 0)
    {
        node->size++;
    }

    return rc;
}

static void * IMAP_get(imap_node_t * node, uint64_t id)
{
    imap_node_t * nd = node->nodes + (id % IMAP_NODE_SZ);
    id /= IMAP_NODE_SZ;

    if (!id)
    {
        return nd->data;
    }

    return (nd->nodes == NULL) ? NULL : IMAP_get(nd, id - 1);
}

static void * IMAP_pop(imap_node_t * node, uint64_t id)
{
    void * data;
    imap_node_t * nd = node->nodes + (id % IMAP_NODE_SZ);
    id /= IMAP_NODE_SZ;

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

    data = (nd->nodes == NULL) ? NULL : IMAP_pop(nd, id - 1);

    if (data != NULL && !--node->size)
    {
        free(node->nodes);
        node->nodes = NULL;
    }

    return data;
}

static void IMAP_walk(imap_node_t * node, imap_cb cb, void * data, int * rc)
{
    imap_node_t * nd;

    for (uint8_t i = 0; i < IMAP_NODE_SZ; i++)
    {
        nd = node->nodes + i;

        if (nd->data != NULL)
        {
            *rc += (*cb)(nd->data, data);
        }

        if (nd->nodes != NULL)
        {
            IMAP_walk(nd, cb, data, rc);
        }
    }
}

