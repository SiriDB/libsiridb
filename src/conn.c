/*
 * conn.c
 *
 *  Created on: Jun 02, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

 #include <conn.h>

siridb_conn_t * siridb__conn_create(
        const char * username,
        const char * password,
        const char * dbname)
{
    siridb_conn_t * conn = (siridb_conn_t *) malloc(sizeof(siridb_conn_t));
    if (conn != NULL)
    {
        conn->username = strdup(username);
        conn->password = strdup(password);
        conn->dbname = strdup(dbname);
        conn->imap = imap_new();
        conn->pid = 0;
        conn->_conn = NULL;

        if (conn->username == NULL ||
            conn->password == NULL ||
            conn->dbname == NULL ||
            conn->imap == NULL)
        {
            siridb_conn_destroy(conn)
            conn = NULL;
        }
    }

    return conn;
}

void siridb__conn_destroy(siridb_conn_t * conn)
{
    free(conn->username);
    free(conn->username);
    free(conn->username);
    if (conn->imap != NULL)
    {
        imap_free(conn->imap, (imap_free_cb) siridb__handle_cancel);
    }
}