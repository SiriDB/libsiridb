# SiriDB Connector (libsiridb) with libuv
This example is written as an extension to libsiridb which we will call libsuv.
It's up to you to decide if you want to install libsuv ([suv.h](#suv.h)/
[suv.c](#suv.c)) as a library or just copy the files in your own project.

THis documentation contains the api exposed by libsuv.

---------------------------------------
  * [API](#api)
    * [suv_buf_t](#suv_buf_t)
    * [siridb_req_t](#siridb_req_t)
    * [siridb_pkg_t](#siridb_pkg_t)
    * [siridb_packer_t](#siridb_packer_t)
    * [siridb_resp_t](#siridb_resp_t)
    * [siridb_series_t](#siridb_series_t)
    * [siridb_point_t](#siridb_point_t)
    * [siridb_timeit_t](#siridb_timeit_t)
    * [siridb_perf_t](#siridb_perf_t)
    * [siridb_select_t](#siridb_select_t)
    * [siridb_list_t](#siridb_list_t)
    * [siridb_show_t](#siridb_show_t)
    * [siridb_item_t](#siridb_item_t)
    * [Miscellaneous functions](#miscellaneous-functions)

---------------------------------------

## API
>Note: libsuv uses some of the public data members which are exposed by libsiridb.
>In most cases libsuv exposes its own public space for user-defined data which
>should be used instead.

### `suv_buf_t`
Buffer required be a connection. Each SiriDB connection must have its own buffer.

#### `suv_buf_t * suv_buf_create(siridb_t * siridb)`
Create and return a new buffer. A `siridb_t` instance is required.

>Warning: Do not use `siridb->data` since it will be overwritten as soon as the
>buffer is used be a connection.

*Public members*
- `void * suv_buf_t.data`: Space for user-defined arbitrary data. libsuv does
not use this field.

#### `void suv_buf_destroy(suv_buf_t * suvbf)`
Cleanup a buffer. Call this function after the connection is closed.

### `suv_write_t`
General write type. Used to communicate with SiriDB.

*Public members*
- `void * suv_write_t.data`: Space for user-defined arbitrary data. libsuv does
not use this field.
- `void * suv_write_t.pkg`: Contains the package to send. (readonly)

#### `void suv_write_destroy(suv_connect_t * connect)`
Cleaunp a write handle. This function should be called from a request
(`siridb_req_t`) callback function.

#### `void suv_write_error(suv_write_t * swrite, int err_code)`
Used to cancel a write handle. This will set the request status to `err_code`,
removes the request from the queue and calls the request callback function.

### `suv_connect_t`
Connect handle. Alias for `suv_write_t`.

#### `suv_connect_t * suv_connect_create(siridb_req_t * req, const char * username, const char * password, const char * dbname)`
Create and return a connection handle. After the connection handle is created,
you must manually bind the handle to `req->data`. This must be done explicit to
make clear that you are also resonsible for handling the cleanup.

Returns `NULL` in case of an memory allocation error.

#### `void suv_connect_destroy(suv_connect_t * connect)`
Cleaunp a connection handle. This function should be called from a request
(`siridb_req_t`) callback function. Alias for `suv_write_destroy()`.

#### `void suv_connect(suv_connect_t * connect, suv_buf_t * buf, uv_tcp_t * tcp, struct sockaddr * addr)`
Connect and authenticate to SiriDB.

>Warning: This function overwrites the members `buf->siridb->data` and `tcp->data` so
>you should not use these properties. Instead the public members `buf->data` and
>`connect->data` are available and save to use.

Example:
```c
#include <uv.h>
#include <libsiridb/siridb.h>
#include "suv.h"

uv_loop_t loop;
uv_tcp_t tcp;

void connect_cb(siridb_req_t * req)
{
    suv_connect_t * connect = (suv_connect_t *) req->data;

    if (req->status) {
        printf("connect or auth failed: %s\n", siridb_strerror(req->status));
    } else {
        // do something with the connection
    }

    /* cleanup connetion handle */
    suv_connect_destroy(connect);

    /* cleanup connection request */
    siridb_req_destroy(req);

    /* lets stop the example */
    uv_close((uv_handle_t *) &tcp, NULL);
}

int main(void)
{
    struct sockaddr_in addr;
    /* initialize uv loop */
    uv_loop_init(&loop);

    /* asume siridb-server is running on localhost and port 9000 */
    uv_ip4_addr("127.0.0.1", 9000, &addr);

    /* create a siridb client */
    siridb_t * siridb = siridb_create();

    /* create a buffer for the connection */
    suv_buf_t * buf = suv_buf_create(siridb);

    /* create a connection request */
    siridb_req_t * req = siridb_req_create(siridb, connect_cb, NULL);

    /* create a connection handle */
    suv_connect_t * connect = suv_connect_create(req, "iris", "siri", "dbtest");

    /* explicit bind the connect handle to the request. (this must be done!) */
    req->data = (void *) connect;

    uv_tcp_init(&loop, &tcp);

    /* Warning: This overwrites tcp->data and siridb->data so do not use these
     *          members yourself. */
    suv_connect(connect, buf, &tcp, (struct sockaddr *) &addr);

    /* run the uv event loop */
    uv_run(&loop, UV_RUN_DEFAULT);

    /* close the loop */
    uv_loop_close(&loop);

    /* cleanup buffer */
    suv_buf_destroy(buf);

    /* cleanup siridb */
    siridb_destroy(siridb);

    return 0;
}
```

### `suv_query_t`
Query handle. Alias for `suv_write_t`.

#### `suv_query_t * suv_query_create(siridb_req_t * req, const char * query)`
Create and return a query handle. After the query handle is created,
you must manually bind the handle to `req->data`. This must be done explicit to
make clear that you are also resonsible for handling the cleanup.

Returns `NULL` in case of an memory allocation error.

#### `void suv_query_destroy(suv_query_t * suvq)`
Cleaunp a query handle. This function should be called from a request
(`siridb_req_t`) callback function. Alias for `suv_write_destroy()`.

#### `void suv_query(suv_query_t * suvq)`
Query SiriDB.

Example
```c
/* first create a request */
siridb_req_t * req = siridb_req_create(siridb, example_cb, NULL);

/* create query handle */
suv_query_t * handle = suv_query_create(req, "select * from 'my-series'");

/* bind query handle to req->data */
req->data = (void *) handle;

/* now run the query, check the callback for the result or errors. */
suv_query(handle);
```

Example callback function:
```c
void example_cb(siridb_req_t * req)
{
    if (req->status != 0) {
        printf("error handling request: %s", siridb_strerror(req->status));
    } else {
        /* get the response */
        siridb_resp_t * resp = siridb_resp_create(req->pkg, NULL);

        // do something with the response...

        // a general cb function could do something based on the response type...
        switch(resp->tp) {
        case SIRIDB_RESP_TP_UNDEF:
        case SIRIDB_RESP_TP_SELECT:
        case SIRIDB_RESP_TP_LIST:
        case SIRIDB_RESP_TP_SHOW:
        case SIRIDB_RESP_TP_COUNT:
        case SIRIDB_RESP_TP_CALC:
        case SIRIDB_RESP_TP_SUCCESS:
        case SIRIDB_RESP_TP_SUCCESS_MSG:
        case SIRIDB_RESP_TP_ERROR:
        case SIRIDB_RESP_TP_ERROR_MSG:
        case SIRIDB_RESP_TP_HELP:
        case SIRIDB_RESP_TP_MOTD:
        case SIRIDB_RESP_TP_DATA: break;
        }

        /* cleanup response */
        siridb_resp_destroy(resp);
    }

    /* destroy handle */
    suv_write_destroy((suv_write_t *) req->data);

    /* destroy request */
    siridb_req_destroy(req);
}
```

### `suv_insert_t`
Insert handle. Alias for `suv_write_t`.

#### `suv_insert_t * suv_insert_create(siridb_req_t * req, siridb_series_t * series[], size_t n)`
Create and return a insert handle. Argument `n` must be equal or smaller than
the number of series in the `series[]` array.

After the insert handle is created, you must manually bind the handle to
`req->data`. This must be done explicit to make clear that you are also
resonsible for handling the cleanup.

Returns `NULL` in case of an memory allocation error.

#### `void suv_insert_destroy(suv_insert_t * insert)`
Cleaunp an insert handle. This function should be called from a request
(`siridb_req_t`) callback function. Alias for `suv_write_destroy()`.

#### `void suv_insert(suv_insert_t * insert`
Insert data into SiriDB.

Example:
```c
/* create an array of series, just one for this example */
siridb_series_t * series[1];

/* create series */
series[0] = siridb_series_create(
    SIRIDB_SERIES_TP_INT64,         /* type int64, could also be double */
    "example-series-name",          /* some name for the series */
    10);                            /* number of points */

/* create some sample points */
for (size_t i = 0; i < series[0]->n; i++) {
    siridb_point_t * point = series[0]->points + i;
    point->ts = (uint64_t) time(NULL) - series[0]->n + i;
    point->via.int64 = (int64_t) i;
}

/* create a request */
siridb_req_t * req = siridb_req_create(siridb, example_cb, NULL);

/* create insert handle */
suv_insert_t * handle = suv_insert_create(req, series, 1);

/* bind the handle to the request */
req->data = (void *) handle;

/* cleanup the series since the series is now packed in handle->pkg */
siridb_series_destroy(series[0]);

/* insert data into siridb, the callback should be checked for errors.
 * (a successful insert has a SIRIDB_RESP_TP_SUCCESS_MSG siridb_resp_t.tp) */
suv_insert(handle);
```