/*
 * siridb.c
 *
 *  Created on: Apr 07, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#include <siridb.h>
#include <errmap.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <pkg.h>
#include <protomap.h>
#include <stdlib.h>
#include <string.h>
#include <thread.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

static void siridb__connect(void * arg);
static void siridb_on_data(siridb_t * conn, ssize_t n);
static void siridb_on_pkg(siridb_t * conn, siridb_pkg_t * pkg);
static void siridb_handle_cancel(siridb_handle_t * handle);

static qp_res_t * siridb_map_get(qp_map_t * map, const char * key);
static siridb_series_tp siridb_points_get_tp(qp_array_t * points);
static siridb_series_t * siridb_series_create(
        siridb_series_tp tp,
        char ** name,
        size_t n);
static void siridb_series_destroy(siridb_series_t * series);

void siridb_handle_init(
        siridb_handle_t * handle,
        siridb_t * conn,
        siridb_cb cb,
        void * arg)
{
    handle->conn = conn;
    handle->cb = cb;
    handle->arg = arg;
    handle->status = ERR_UNFINISHED;
    handle->pkg = NULL;
}

static void siridb_handle_cancel(siridb_handle_t * handle)
{
    handle->status = ERR_CANCELLED;
    handle->cb(handle);
}

int siridb_init(
        siridb_t * conn,
        struct sockaddr_in * addr,
        const char * username,
        const char * password,
        const char * dbname)
{
    conn->addr = addr;
    conn->username = strdup(username);
    conn->password = strdup(password);
    conn->dbname = strdup(dbname);
    conn->imap = imap_new();
    conn->buf = malloc(4096);
    conn->buf_sz = 4096;
    conn->buf_len = 0;
    conn->pid = 0;


    if (siridb_mutex_init(&conn->mutex) ||
        conn->username == NULL ||
        conn->password == NULL ||
        conn->dbname == NULL ||
        conn->imap == NULL)
    {
        return -1;
    }
    return 0;
}

void siridb_destroy(siridb_t * conn)
{
    if (conn->imap != NULL)
    {
        imap_free(conn->imap, (imap_free_cb) siridb_handle_cancel);
    }
    if (conn->buf != NULL)
    {
        free(conn->buf);
    }
    if (conn->username != NULL)
    {
        free(conn->username);
    }
    if (conn->password != NULL)
    {
        free(conn->password);
    }
    if (conn->dbname != NULL)
    {
        free(conn->dbname);
    }

    siridb_mutex_destroy(&conn->mutex);
}

void siridb_read_data(void * arg)
{
    siridb_t * conn = (siridb_t *) arg;
    ssize_t n;

    while (conn->sockfd >= 0)
    {
        n = read(
                conn->sockfd,
                conn->buf + conn->buf_len,
                conn->buf_sz - conn->buf_len);
        siridb_on_data(conn, n);
    }
}

static void siridb_on_data(siridb_t * conn, ssize_t n)
{
    size_t total_sz;
    siridb_pkg_t * pkg;

    if (n < 0)
    {
        // TODO: error...
        printf("error....n = %zd\n", n);
        abort();
        return;
    }

    conn->buf_len += n;

    if (conn->buf_len < SIRIDB_PKG_SZ)
    {
        return;
    }

    pkg = (siridb_pkg_t *) conn->buf;
    if ((pkg->tp ^ 255) != pkg->checkbit || pkg->len > SIRIDB_PKG_MAX_SZ)
    {
        // TODO: error...
        printf("error....pkg check");
        return;
    }

    total_sz = pkg->len + SIRIDB_PKG_SZ;

    if (conn->buf_len >= total_sz)
    {
        siridb_on_pkg(conn, pkg);

        if (conn->buf_len == total_sz)
        {
            conn->buf_len = 0;
        }
        else
        {
            conn->buf_len -= total_sz;
            memmove(conn->buf, conn->buf + total_sz, conn->buf_len);
            siridb_on_data(conn, 0);
        }
        return;
    }

    if (total_sz > conn->buf_sz)
    {
        char * tmp;
        tmp = (char *) realloc(conn->buf, total_sz);
        if (tmp == NULL)
        {
            // TODO: error...
            printf("error....re-allocation");
            return;
        }
        conn->buf = tmp;
        conn->buf_sz = total_sz;
    }
}


static siridb_series_tp siridb_points_get_tp(qp_array_t * points)
{
    qp_array_t * point;

    if (!points->n)
    {
        /* return integer type in case the series has no points */
        return SIRIDB_SERIES_TP_INT64;
    }

    if (points->values[0].tp != QP_RES_ARRAY ||
        (point = points->values[0].via.array)->n != 2)
    {
        return -1;
    }

    switch (point->values[1].tp)
    {
    case QP_RES_INT64:
        return SIRIDB_SERIES_TP_INT64;
    case QP_RES_REAL:
        return SIRIDB_SERIES_TP_REAL;
    case QP_RES_STR:
        return SIRIDB_SERIES_TP_STR;
    case QP_RES_MAP:
    case QP_RES_ARRAY:
    case QP_RES_BOOL:
    case QP_RES_NULL:
    default:
        return -1;
    }
}

/*
 * This binds *name to the series->name and makes *name NULL when this function
 * is successful.
 */
static siridb_series_t * siridb_series_create(
        siridb_series_tp tp,
        char ** name,
        size_t n)
{
    siridb_series_t * series = (siridb_series_t *) malloc(
            sizeof(siridb_series_t) + n * sizeof(siridb_point_t));

    if (series != NULL)
    {
        series->tp = tp;

        /* move name rather than copy to avoid memory allocations */
        series->name = *name;
        *name = NULL;

        series->n = n;

        if (series->tp == SIRIDB_SERIES_TP_STR)
        {
            /* initialize string series with NULL pointers */
            size_t i;
            for (i = 0; i < n; i++)
            {
                series->points[i].via.str = NULL;
            }
        }
    }

    return series;
}

static void siridb_series_destroy(siridb_series_t * series)
{
    if (series->tp == SIRIDB_SERIES_TP_STR)
    {
        size_t i;
        for (i = 0; i < series->n; i++)
        {
            free(series->points[i].via.str);
        }
    }
    free(series->name);
    free(series);
}




static void siridb_on_pkg(siridb_t * conn, siridb_pkg_t * pkg)
{

    siridb_handle_t * handle =
            (siridb_handle_t *) imap_pop(conn->imap, (uint64_t) pkg->pid);

    if (handle == NULL)
    {
        //TODO: error, handle not found...
        printf("error....handle not found");
        return;
    }

    handle->pkg = siridb_pkg_dup(pkg);
    printf("Found handle...");
    handle->status = (handle->pkg == NULL) ? ERR_MEM_ALLOC : 0;
    printf("Status ... %d (%u)\n", handle->status, pkg->pid);
    conn->cb(handle);
}

int siridb_connect(siridb_handle_t * handle)
{
    siridb_thread_t thread;
    return siridb_thread_create(&thread, siridb__connect, handle);
}

int siridb_send(siridb_handle_t * handle, siridb_pkg_t * pkg)
{
    siridb_handle_t * prev;
    size_t nwritten, sz = pkg->len + SIRIDB_PKG_SZ;
    ssize_t n;
    prev = (siridb_handle_t *) imap_pop(handle->conn->imap, pkg->pid);
    if (prev != NULL)
    {
        // error, a previous handle with this pid is found
        prev->status = ERR_NO_REPLY;
        prev->cb(prev);
    }

    if (imap_add(handle->conn->imap, pkg->pid, (void *) handle) < 0)
    {
        handle->status = ERR_MEM_ALLOC;
        handle->cb(handle);
        return -1;
    }

    for (nwritten = 0; nwritten < sz; )
    {
        do
        {
            n = write(handle->conn->sockfd, pkg, sz);
        }
        while (n == -1 && errno == EINTR);

        if (n != -1)
        {
            nwritten += n;
            continue;
        }
    }

    return 0;
}

static void siridb__connect(void * arg)
{

    struct hostent *hp;
    struct sockaddr_in sin;
    memset(&sin, '\0', sizeof(sin));

    int port = 9000;
//    int rc;

    hp = gethostbyname("localhost");
    if ( hp == NULL ) {
        fprintf(stderr, "host not found (%s)\n", "localhost");
        exit(1);
    }



    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);


    siridb_handle_t * handle = (siridb_handle_t *) arg;
    siridb_t * conn = handle->conn;
    qp_packer_t * packer;

    if ((conn->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        /* TODO: check errno for more details */
        handle->status = ERR_SOCK_FD;
        conn->cb(handle);
    }
    else if (connect(conn->sockfd, (struct sockaddr *) &sin, sizeof(sin)) < 0)
    {
        /* TODO: check errno for more details */
        int e = errno;
        printf("error: %s\n", strerror(e));
        handle->status = ERR_SOCK_CONNECT;
        conn->cb(handle);
    }
    else if (siridb_thread_create(&conn->thread, siridb_read_data, conn))
    {
        // TODO: error handling...
        handle->status = ERR_THREAD_START;
        conn->cb(handle);
    }
    else if (
        (packer = qp_packer_create(512)) == NULL ||
        qp_add_array(&packer) ||
        qp_add_raw(packer, conn->username, strlen(conn->username)) ||
        qp_add_raw(packer, conn->password, strlen(conn->password)) ||
        qp_add_raw(packer, conn->dbname, strlen(conn->dbname)) ||
        qp_close_array(packer))
    {
        handle->status = ERR_MEM_ALLOC;
        conn->cb(handle);
    }
    else
    {
        siridb_pkg_t * pkg = sirinet_pkg_new(
                CprotoReqAuth,
                conn->pid++,
                packer->len,
                packer->buffer);
        siridb_send(handle, pkg);
    }

    pthread_exit(NULL);
}

void siridb_query(siridb_handle_t * handle, const char * query)
{
    qp_packer_t * packer;
    if (
        (packer = qp_packer_create(512)) == NULL ||
        qp_add_array(&packer) ||
        qp_add_raw(packer, query, strlen(query)) ||
        qp_close_array(packer))
    {
        abort();
    }
    else
    {
        siridb_pkg_t * pkg = sirinet_pkg_new(
                CprotoReqQuery,
                handle->conn->pid++,
                packer->len,
                packer->buffer);
        if (pkg == NULL)
        {
            abort();
        }
        siridb_send(handle, pkg);
    }
}

static qp_res_t * siridb_map_get(qp_map_t * map, const char * key)
{
    size_t i;

    for (i = 0; i < map->n; i++)
    {
        if (map->keys[i].tp == QP_RES_STR &&
            strcmp(map->keys[i].via.str, key) == 0)
        {
            return map->values + i;
        }
    }

    return NULL;
}
