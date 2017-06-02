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
#include <packer.h>
#include <pkg.h>
#include <protomap.h>
#include <stdlib.h>
#include <string.h>
#include <thread.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

void siridb_send(siridb_handle_t * handle, siridb_pkg_t * pkg)
{
    int rc = imap_add(handle->conn->imap, pkg->pid, (void *) handle);

    if (rc == ERR_MEM_ALLOC)
    {
        handle->status = ERR_MEM_ALLOC;
        handle->cb(handle);
        return;
    }

    if (rc == ERR_OCCUPIED)
    {
        siridb_handle_t * prev;
        prev = (siridb_handle_t *) imap_pop(handle->conn->imap, pkg->pid);
        assert (prev != NULL);

        // error, a previous handle with this pid is found
        prev->status = ERR_NO_REPLY;
        prev->cb(prev);
    }

    rc = siridb__write(handle, pkg);

    if (!rc)
    {
        handle->status = rc;
        imap_pop(handle->conn->imap, pkg->pid);
        handle->cb(handle);
    }
}

void siridb_query(siridb_handle_t * handle, const char * query)
{
    siridb_packer_t * packer;
    if (
        (packer = siridb_packer_create(512)) == NULL ||
        qp_add_array(&packer) ||
        qp_add_raw(packer, query, strlen(query)) ||
        qp_close_array(packer))
    {
        handle->status = ERR_MEM_ALLOC;
        handle->cb(handle);
    }
    else
    {
        siridb_pkg_t * pkg = siridb_packer_2pkg(
                packer,
                handle->conn->pid++,
                CprotoReqQuery);
        siridb_send(handle, pkg);
        free(pkg);
    }
}

void siridb__on_pkg(siridb_conn_t * conn, siridb_pkg_t * pkg)
{
    siridb_handle_t * handle =
            (siridb_handle_t *) imap_pop(conn->imap, (uint64_t) pkg->pid);

    if (handle == NULL)
    {
        printf("error: handle not found, most likely it was cancelled\n");
        return;
    }

    handle->pkg = siridb_pkg_dup(pkg);
    handle->status = (handle->pkg == NULL) ? ERR_MEM_ALLOC : 0;
    handle->cb(handle);
}

// id siridb_read_data(void * arg)
// {
//     siridb_t * conn = (siridb_t *) arg;
//     ssize_t n;

//     while (conn->sockfd >= 0)
//     {
//         n = read(
//                 conn->sockfd,
//                 conn->buf + conn->buf_len,
//                 conn->buf_sz - conn->buf_len);
//         siridb_on_data(conn, n);
//     }
//     pthread_exit(NULL);
// }

// static void siridb_on_data(siridb_t * conn, ssize_t n)
// {
//     size_t total_sz;
//     siridb_pkg_t * pkg;

//     if (n < 0)
//     {
//         printf("error: while reading data, closing connection\n");
//         siridb_destroy(conn);
//         return;
//     }

//     conn->buf_len += n;

//     if (conn->buf_len < SIRIDB_PKG_SZ)
//     {
//         return;
//     }

//     pkg = (siridb_pkg_t *) conn->buf;
//     if ((pkg->tp ^ 255) != pkg->checkbit || pkg->len > SIRIDB_PKG_MAX_SZ)
//     {
//         printf("error: received an invalid pkg, closing connection\n");
//         siridb_destroy(conn);
//         return;
//     }

//     total_sz = pkg->len + SIRIDB_PKG_SZ;

//     if (conn->buf_len >= total_sz)
//     {
//         siridb_on_pkg(conn, pkg);

//         if (conn->buf_len == total_sz)
//         {
//             conn->buf_len = 0;
//         }
//         else
//         {
//             conn->buf_len -= total_sz;
//             memmove(conn->buf, conn->buf + total_sz, conn->buf_len);
//             siridb_on_data(conn, 0);
//         }
//         return;
//     }

//     if (total_sz > conn->buf_sz)
//     {
//         char * tmp;
//         tmp = (char *) realloc(conn->buf, total_sz);
//         if (tmp == NULL)
//         {
//             printf("error: while calling realloc, closing connection\n");
//             siridb_destroy(conn);
//             return;
//         }
//         conn->buf = tmp;
//         conn->buf_sz = total_sz;
//     }
// }

// int siridb_connect(siridb_handle_t * handle)
// {
//     return siridb_thread_create(
//             &handle->conn->connthread,
//             siridb__connect,
//             handle);
// }

// static void siridb__connect(void * arg)
// {
//     struct hostent *hp;
//     struct sockaddr_in sin;
//     memset(&sin, '\0', sizeof(sin));

//     int port = 9000;
// //    int rc;

//     hp = gethostbyname("localhost");
//     if ( hp == NULL ) {
//         fprintf(stderr, "host not found (%s)\n", "localhost");
//         exit(1);
//     }

//     sin.sin_family = AF_INET;
//     sin.sin_port = htons(port);
//     memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);


//     siridb_handle_t * handle = (siridb_handle_t *) arg;
//     siridb_t * conn = handle->conn;
//     qp_packer_t * packer;

//     if ((conn->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
//     {
//         /* TODO: check errno for more details */
//         handle->status = ERR_SOCK_FD;
//         conn->cb(handle);
//     }
//     else if (connect(conn->sockfd, (struct sockaddr *) &sin, sizeof(sin)) < 0)
//     {
//         /* TODO: check errno for more details */
//         int e = errno;
//         printf("error: %s\n", strerror(e));
//         handle->status = ERR_SOCK_CONNECT;
//         conn->cb(handle);
//     }
//     else if (siridb_thread_create(&conn->thread, siridb_read_data, conn))
//     {
//         // TODO: error handling...
//         handle->status = ERR_THREAD_START;
//         conn->cb(handle);
//     }
//     else if (
//         (packer = siridb_packer_create(512)) == NULL ||
//         qp_add_array(&packer) ||
//         qp_add_raw(packer, conn->username, strlen(conn->username)) ||
//         qp_add_raw(packer, conn->password, strlen(conn->password)) ||
//         qp_add_raw(packer, conn->dbname, strlen(conn->dbname)) ||
//         qp_close_array(packer))
//     {
//         handle->status = ERR_MEM_ALLOC;
//         conn->cb(handle);
//     }
//     else
//     {
//         siridb_pkg_t * pkg = sirinet_pkg_new(
//                 CprotoReqAuth,
//                 conn->pid++,
//                 packer->len,
//                 packer->buffer);
//         siridb_send(handle, pkg);
//         free(pkg);
//         qp_packer_destroy(packer);
//     }

//     pthread_exit(NULL);
// }



