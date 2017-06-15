/*
 * pkg.c
 *
 *  Created on: Apr 07, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#include <siridb.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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
        siridb__pkg_set_checkbit(pkg);

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

siridb_pkg_t * siridb_pkg_query(uint16_t pid, const char * query)
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

siridb_pkg_t * siridb_pkg_series(
    uint16_t pid,
    siridb_series_t * series[],
    size_t n)
{
    int rc = 0;

    siridb_packer_t * packer = siridb_packer_create(4096);
    if (packer == NULL)
    {
        return NULL;
    }

    rc += qp_add_map(&packer);
    for (size_t i = 0; i < n; i ++)
    {
        siridb_series_t * s = series[i];
        rc += qp_add_raw(packer, s->name, strlen(s->name)); /* series name */
        rc += qp_add_array(&packer); /* add points array */
        for (size_t p = 0; p < s->n; p++)
        {
            siridb_point_t * point = s->points + p;
            rc += qp_add_array(&packer); /* add point array */
            rc += qp_add_int64(packer, (int64_t) point->ts);
            switch(s->tp)
            {
            case SIRIDB_SERIES_TP_INT64:
                rc += qp_add_int64(packer,point->via.int64); break;
            case SIRIDB_SERIES_TP_REAL:
                rc += qp_add_double(packer,point->via.real); break;
            case SIRIDB_SERIES_TP_STR:
                rc += qp_add_raw(
                    packer,
                    point->via.str,
                    strlen(point->via.str)); break;
            default:
                assert (0); /* unknown series type */
            }
            rc += qp_close_array(packer); /* close point array */
        }
        rc += qp_close_array(packer); /* close points array */
    }

    rc += qp_close_map(packer);
    if (rc)
    {
        siridb_packer_destroy(packer);
        return NULL;
    }

    return siridb_packer_2pkg(packer, pid, CprotoReqInsert);
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
