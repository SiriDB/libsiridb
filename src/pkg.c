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

siridb_pkg_t * sirinet_pkg_create(
        uint16_t pid,
        uint8_t tp,
        const unsigned char * data,
        uint32_t len)
{
    siridb_pkg_t * pkg =
            (siridb_pkg_t *) malloc(sizeof(siridb_pkg_t) + len);

    if (pkg != NULL)
    {
        pkg->pid = pid;
        pkg->len = len;
        pkg->tp = tp;
        siridb_pkg_set_checkbit(pkg);

        if (data != NULL)
        {
            memcpy(pkg->data, data, len);
        }
    }

    return pkg;
}

siridb_pkg_t * siridb_pkg_auth(
    uint16_t pid,
    const char * username,
    const char * password,
    const char * dbname)
{
    siridb_packer_t * packer = siridb_packer_create(512);
    if (packer == NULL)
    {
        return NULL;
    }

    if (qp_add_array(&packer) ||
        qp_add_raw(packer, username, strlen(username)) ||
        qp_add_raw(packer, password, strlen(password)) ||
        qp_add_raw(packer, dbname, strlen(dbname)) ||
        qp_close_array(packer))
    {
        siridb_packer_destroy(packer);
        return NULL;
    }

    return siridb_packer_2pkg(packer, pid, CprotoReqAuth);
}

siridb_pkg_t * siridb_pkg_query(
    uint16_t pid,
    const char * query)
{
    siridb_packer_t * packer = siridb_packer_create(512);
    if (packer == NULL)
    {
        return NULL;
    }

    if (qp_add_array(&packer) ||
        qp_add_raw(packer, query, strlen(query)) ||
        qp_close_array(packer))
    {
        siridb_packer_destroy(packer);
        return NULL;
    }

    return siridb_packer_2pkg(packer, pid, CprotoReqQuery);
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
