#include <stdio.h>
#include <uv.h>
#include <qpack.h>
#include <siridb/siridb.h>




int main(void)
{
    uv_tcp_t tcp;
    uv_loop_t loop;
    uv_loop_init(loop);

    uv_ip4_addr(SERVER, PORT, &addr);

    siridb_t * siridb = siridb_create();
    /* handle siridb == NULL */

    suv_buf_t * suvbf = suv_buf_create(siridb);
    /* handle suvbf == NULL */

    uv_connect_t * uvreq = (uv_connect_t *) malloc(sizeof(uv_connect_t));
    /* handle uvreq == NULL */

    tcp.data = (void *) suvbf;

    uv_tcp_init(loop, &tcp);
    uv_tcp_connect(uvreq, &tcp, (struct sockaddr *) &addr, connect_cb);

    uv_run(loop, UV_RUN_DEFAULT);
    close_handlers();

    uv_loop_close(loop);
    return 0;
}