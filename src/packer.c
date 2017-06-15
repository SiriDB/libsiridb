/*
 * conn.c
 *
 *  Created on: Jun 02, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#include <packer.h>
#include <stdlib.h>
#include <assert.h>

/*
 * Returns a siridb_packer_t which is a wrapped qp_packer_t which can be
 * transformed to a siridb_pkg_t.
 */
siridb_packer_t * siridb_packer_create(size_t alloc_size)
{
    assert (alloc_size >= sizeof(siridb_pkg_t));

    qp_packer_t * packer = qp_packer_create(alloc_size);
    if (packer != NULL)
    {
        packer->len = sizeof(siridb_pkg_t);
    }

    return packer;
}

/*
 * Do not call packer destroy after calling siridb_packer_2pkg.
 */
void siridb_packer_destroy(siridb_packer_t * packer)
{
    qp_packer_destroy(packer);
}

/*
 * Returns a siridb_pkg_t from a siridb_packer_t.
 *
 * Call 'free' to destroy the returned siridb_pkg_t and do not destroy the
 * packer anymore since this is handled here.
 */
siridb_pkg_t * siridb_packer_2pkg(
    siridb_packer_t * packer,
    uint16_t pid,
    uint8_t tp)
{
    siridb_pkg_t * pkg = (siridb_pkg_t *) packer->buffer;

    pkg->pid = pid;

    /* set package type and the correct check bit */
    pkg->tp = tp;
    siridb__pkg_set_checkbit(pkg);

    /* set pkg length without the header size */
    pkg->len = packer->len - sizeof(siridb_pkg_t);

    /* Free the packer, not the buffer */
    free(packer);

    return pkg;
}