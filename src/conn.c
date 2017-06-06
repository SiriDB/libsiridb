/*
 * conn.c
 *
 *  Created on: Jun 02, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

 #include <conn.h>
 #include <string.h>
 #include <stdlib.h>
 #include <handle.h>

siridb_conn_t * siridb_conn_create(void * data)
{
    siridb_conn_t * conn = (siridb_conn_t *) malloc(sizeof(siridb_conn_t));
    if (conn != NULL)
    {
        conn->imap = imap_create();
        conn->pid = 0;
        conn->data = data;

        if (conn->imap == NULL)
        {
            siridb_conn_destroy(conn);
            conn = NULL;
        }
    }

    return conn;
}

void siridb_conn_destroy(siridb_conn_t * conn)
{
    if (conn->imap != NULL)
    {
        imap_destroy(conn->imap, (imap_free_cb) siridb_handle_cancel);
    }
}