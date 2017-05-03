/*
 * pkg.c
 *
 *  Created on: Apr 07, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#include <pkg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

siridb_pkg_t * sirinet_pkg_new(
        uint8_t tp,
        uint16_t pid,
        uint32_t len,
        const unsigned char * data)
{
    siridb_pkg_t * pkg =
            (siridb_pkg_t *) malloc(sizeof(siridb_pkg_t) + len);

    if (pkg != NULL)
    {
        pkg->len = len;
        pkg->pid = pid;
        pkg->tp = tp;
        pkg->checkbit = pkg->tp ^ 255;

        if (data != NULL)
        {
            memcpy(pkg->data, data, len);
        }
    }

    return pkg;
}

/*
 * Returns a copy of package allocated using malloc().
 * In case of an allocation error, NULL is returned.
 */
siridb_pkg_t * siridb_pkg_dup(siridb_pkg_t * pkg)
{
    size_t size = sizeof(siridb_pkg_t) + pkg->len;
    siridb_pkg_t * dup = (siridb_pkg_t *) malloc(size);
    if (dup != NULL)
    {
        memcpy(dup, pkg, size);
    }
    return dup;
}
