# SiriDB Connector C (libsiridb)
SiriDB Connector C is a library which can be used to communicate with SiriDB
using the C program language. This library contains useful functions but does
not handle the connection itself. When using [libuv](http://libuv.org/) we do
have a complete [example](libuv_example/README.md) which can be used easily for
any project.

Siridb can handle multiple queries and/or inserts on a single connection
simultaniously. The order in which results are returned is not defined and as
soon as a request is finished the result will be returned. This means a client
should keep track of each request. We do this by assigning a pid to each
request. This pid is an unsigned 16bit integer value and the client is
responsable for chosing a unique pid. SiriDB simple returns the same pid in its
response so the client then should able to map the result to the request.
Assigning a unique pid to a request and mapping a response to the request is
the resposibility of this library.

---------------------------------------
  * [Installation](#installation)
  * [Example (libuv implementation)](#example-libuv-implementation)
  * [API](#api)
    * [siridb_t](#siridb_t)
    * [siridb_req_t](#siridb_req_t)
    * [siridb_pkg_t](#siridb_pkg_t)
    * [siridb_packer_t](#siridb_packer_t)
    * [siridb_resp_t](#siridb_resp_t)
    * [siridb_series_t](#siridb_series_t)
    * [siridb_point_t](#siridb_point_t)
    * [Miscellaneous functions](#miscellaneous-functions)

---------------------------------------

## Installation
Install debug or release version, in this example we will install the release version.
```
$ cd Release
```

Compile libsiridb
```
$ make all
```

Install libsiridb
```
$ sudo make install
```

> Note: run `sudo make uninstall` for removal.

## Example (libuv implementation)
This project contains an example of how libsiridb can be used with libuv. The
example is created as an extend to this api using [suv.h](libuv_example/suv.h)
and [suv.c](libuv_example/suv.c) which is explained
[here](libuv_example/README.md). Look at [main.c](#libuv_example/main.c) for an
example of how to use libsiridb and the libuv implementation.

## API
### siridb_t
SiriDB Client type. Pending request are stored in a queue on this object until
a response is received or the request is cancelled.

*Public members*
- `void * siridb_t.data`: Space for user-defined arbitrary data. libsiridb does
not use this field.

#### `siridb_t * siridb_create(void)`
Creates a new SiriDB Client instance. In case of an error, NULL will be returned.

#### `void siridb_destroy(siridb_t * siridb)`
Cleanup a SiriDB Client instance. In case the queue has pending request then each
request will be cancelled. This means the requests status will be set to
`ERR_CANCELLED` and the callback function will be called.

#### `void siridb_on_pkg(siridb_t * siridb, siridb_pkg_t * pkg)`
Should be called when a package is received on a SiriDB connection. This
function then takes the request from the queue and calls the callback function.
If this function is able to run the callback, 0 is returned. In case the
request cannot be found, `ERR_NOT_FOUND` is returned.

#### `size_t siridb_queue_len(siridb_t * siridb)`
Returns the numbers of requests in the queue.

### siridb_req_t
SiriDB Request type. For each request (connecting, queries and inserts) to
SiriDB a request is required. The request gets a pid assigned and will be
stored in a `siridb_t.queue` until the request is cancelled or a response
package with the corresponding pid is handled by `siridb_on_pkg()`.

*Public members*
- `void * siridb_req_t.data`: Space for user-defined arbitrary data. libsiridb does
not use this field.
- `int siridb_req_t.status`: Current status of the request. Status is 0 or an error code. (readonly)
- `uint16_t siridb_req_t.pid`: Assigned pid. (readonly)
- `siridb_pkg_t * siridb_req_t.pkg`: Raw package after response package is handled by `siridb_on_pkg()`. (readonly)

This example shows how `siridb_req_t` should be used with `siridb_t`:
```c
#include <stdio.h>
#include <libsiridb/siridb.h>

void some_cb(siridb_req_t * req)
{
    if (req->status) {
        printf("error: %s\n", siridb_strerror(req->status));
    } else {
        // do something
    }
    siridb_req_destroy(req);
}

int main(void)
{
    /* no error checking to keep the example simple */
    siridb_t * siridb = siridb_create();
    siridb_req_t * req = siridb_req_create(siridb, some_cb, NULL);

    /* Here you should usually connect to SiriDB and use the request for
     * authentication, a query or insert. In case you just run this
     * example, the request will be cancelled by siridb_destroy(). */

    siridb_destroy(siridb);
}
```

#### `siridb_req_t * siridb_req_create(siridb_t * siridb, siridb_cb cb, int * rc)`
Creates and returns a new request. A pid is assigned to the request which is
unique in `siridb.queue`. A callback function is required and the
`siridb_req_t.status` property should be checked by the callback function for
errors. The initial status is `ERR_PENDING`. This function returns `NULL` in
case of an error and optionally `rc` can be used for more information about the
error. If `rc` is not `NULL` then the value is set to 0 if successful,
`ERR_MEM_ALLOC` if memory could not be allocated or `ERR_OCCUPIED` if no unique
pid could be assigned to the request (only happens if 65536 request are pending).

#### `void siridb_req_destroy(siridb_req_t * req)`
Cleanup request. This function should only be used by the callback function or
after the callback function is called. Use `siridb_req_cancel()` if you want to
cancel a request before a response is received.

#### `void siridb_req_cancel(siridb_req_t * req)`
Cancel a request. Use this function to cancel a created request. This function
will remove the request from the queue (if exists), sets the request status to
`ERR_CANCELLED` and calls the callback function. The callback function is
responsable for destroying the request.

### `siridb_pkg_t`
SiriDB Package type. When communicating with SiriDB, both sending and receiving
is done with packages.

*Members (in order)*
- `uint32_t siridb_pkg_t.len`: Length of `siridb_pkg_t.data[]`. (readonly)
- `uint16_t siridb_pkg_t.pid`: Package identifiers. (readonly)
- `uint8_t siridb_pkg_t.tp`: Package type. (readonly)

  *Request types*
  - `CprotoReqQuery`: Request query.
  - `CprotoReqInsert`: Request insert.
  - `CprotoReqAuth`: Request authentication.
  - `CprotoReqPing`: Request ping.
  - `CprotoReqAdmin`: Request service task.

  *Successful response types*
  - `CprotoResQuery`: Response to successful query.
  - `CprotoResInsert`: Response to successful insert.
  - `CprotoResAuthSuccess`: Response to successful authentication. (no data)
  - `CprotoResAck`: Response ACK. (no data)
  - `CprotoResInfo`: Do not use. (only used by siridb-server)
  - `CprotoResFile`: Do not use. (only used by siridb-server)
  - `CprotoAckAdmin`: Response on successful service task. (no data)
  - `CprotoAckAdminData`: Response with data on successful service task.

  *Error response types*
  - `CprotoErrMsg`: Error with message.
  - `CprotoErrQuery`: Returned when the query is invalid.
  - `CprotoErrInsert`: Returned when the insert-data is invalid.
  - `CprotoErrServer`: Request could not be handled by the server.
  - `CprotoErrPool`: Request could not be handled because a required pool is unavailable.
  - `CprotoErrUserAccess`: Database user has not enough privileges to handle request.
  - `CprotoErr`: General error. (no data)
  - `CprotoErrNotAuthenticated`: Connection is not authenticated. (no data)
  - `CprotoErrAuthCredentials`: Credentials are invalid. (no data)
  - `CprotoErrAuthUnknownDb`: Database is unknown. (no data)
  - `CprotoErrLoadingDb`: Do not use. (only used by siridb-server)
  - `CprotoErrFile`: Do not use. (only used by siridb-server)
  - `CprotoErrAdmin`: Service error with message.
  - `CprotoErrAdminInvalidRequest`: Invalid service request. (no data)
- `uint8_t siridb_pkg_t.checkbit`: Checkbit. (readonly)
- `unsigned char siridb_pkg_t.data[]`: Empty or content serialized using libqpack. (readonly)

#### `siridb_pkg_t * sirinet_pkg_create(uint16_t pid, uint8_t tp, const unsigned char * data, uint32_t len)`
Creates and returns a new package. For the pid you should create a
`siridb_req_t`. In case of an error, `NULL` is returned.

Example creating a ping request package:
```c
/* error handling is omitted to keep the example short */
siridb_req_t * ping_req;
siridb_pkg_t * ping_pkg;
ping_req = siridb_req_create(siridb, on_ping_cb, NULL);
ping_pkg = sirinet_pkg_create(ping_req->pid, CprotoReqPing, NULL, 0);
/* now you should write (char *) ping_pkg to the siridb socket connection */
```

>Note: for the most frequently used packages we have specialized functions:
>`siridb_pkg_auth()`, `siridb_pkg_query()` and `siridb_pkg_series()`.

#### `siridb_pkg_t * siridb_pkg_auth(uint16_t pid, const char * username, const char * password, const char * dbname)`
Creates and returns a new package for authenticating with SiriDB. Usually you
should send an authentication package after creating a socket connection to
SiriDB so the connection becomes *authenticated*. Returns `NULL` in case of a
memory allocation error.

#### `siridb_pkg_t * siridb_pkg_query(uint16_t pid, const char * query)`
Creates and returns a new package for querying SiriDB. Returns `NULL` in case
of a memory allocation error.

#### `siridb_pkg_t * siridb_pkg_series(uint16_t pid, siridb_series_t * series[], size_t n)`
Creates and returns a new package for inserting data into SiriDB. The content
for the packge is created from an array of [siridb_series_t](#siridb_series_t).
Argument `n` is the number of series which are packed and must be equal or smaller
than `series[]`. Returns `NULL` in case of a memory allocation error.

#### `siridb_pkg_t * siridb_pkg_dup(siridb_pkg_t * pkg)`
Duplicate a pacakge. Returns `NULL` in case of a memory allocation error.

#### `bool siridb_pkg_check_bit(siridb_pkg_t * pkg)`
Macro function for checking if a package has a valid checkbit. When a package
header is received, this functions should be used to check if the package is
valid. Returns 1 (TRUE) if the package is valid or 0 (FALSE) if not.

### `siridb_packer_t`
Alias for `qp_packer_t`. We use a own defined type since `siridb_packer_t`
created with `siridb_packer_create()` reserve extra space at the beginning of
size `sizeof(siridb_pkg_t)`. This extra space make a `siridb_packer_t` instance
work with any `qp_packer_t` function except `qp_packer_print()`.

#### `siridb_packer_t * siridb_packer_create(size_t alloc_size)`
Like `qp_packer_create` except that the minimal `alloc_size` size is
`sizeof(siridb_pkg_t)`.

#### `void siridb_packer_destroy(siridb_packer_t * packer)`
Cleanup `siridb_packer_t`.
>Note: Do not use this function after calling `siridb_packer_2pkg()`.

#### `siridb_pkg_t * siridb_packer_2pkg(siridb_packer_t * packer, uint16_t pid, uint8_t tp)`
Creates and returns a new package from a `siridb_packer_t`.
>Note: The packer will be destroyed and can not be used after calling this
>function. No new memory will be allocated by this function because the
>`siridb_pkg_t` is created from the `siridb_packer_t.buffer`.

### `siridb_resp_t`
SiriDB Response type. A response type is created from a `siridb_pkg_t` and
unpacks the raw qpack data to a more easy-to-use data object.

*Public members*
- `siridb_resp_tp siridb_resp_t.tp`: Response object type.
  - `SIRIDB_RESP_TP_UNDEF` *(value = 0, undefined response type)*
  - `SIRIDB_RESP_TP_SELECT`
  - `SIRIDB_RESP_TP_LIST`
  - `SIRIDB_RESP_TP_SHOW`
  - `SIRIDB_RESP_TP_COUNT`
  - `SIRIDB_RESP_TP_CALC`
  - `SIRIDB_RESP_TP_SUCCESS`
  - `SIRIDB_RESP_TP_SUCCESS_MSG`
  - `SIRIDB_RESP_TP_ERROR`
  - `SIRIDB_RESP_TP_ERROR_MSG`
  - `SIRIDB_RESP_TP_HELP`
  - `SIRIDB_RESP_TP_MOTD` *(used only by siridb-server DEBUG-release)*
  - `SIRIDB_RESP_TP_DATA` *(used only for admin/service task response)*
- `siridb_resp_via_t siridb_resp_t.via`: Response data.
  - `siridb_select_t * select`
  - `siridb_list_t * list`
  - `siridb_show_t * show`
  - `uint64_t count`
  - `uint64_t calc`
  - `char * success`
  - `char * success_msg`
  - `char * error`
  - `char * error_msg`
  - `char * help`
  - `char * motd`
  - `qp_res_t * data`
- `siridb_timeit_t * siridb_resp_t.timeit`: Optional timeit info or `NULL`.

#### `siridb_resp_t * siridb_resp_create(siridb_pkg_t * pkg, int * rc)`
Creates and returns a response. A `siridb_req_t` callback function should
check for the request status and when this status is zero then the
`siridb_req_t.pkg` property can be used to create a nice response object.
In case of an error `NULL` is returned and an optional `rc` argument can be
used to get more detailed information about the error.

Example:
```c
void some_callback_func(siridb_req_t * req)
{
    if (req->status != 0) {
        printf("error: %s", siridb_strerror(req->status));
    } else {
        int rc;
        // we known for sure req->pkg is not NULL since req-status is 0
        siridb_resp_t * resp = siridb_resp_create(req->pkg, &rc);
        if (rc) {
            printf("error creating response type : %s", siridb_strerror(rc));
        } else {
            // do something with the response

            /* cleanup response */
            siridb_resp_destroy(resp);
        }
    }

    /* cleanup request */
    siridb_req_destroy(req);
}
```

### `siridb_series_t`
SiriDB Series type. This type is used when a `siridb_resp_t` is created from
a `select` query statement to SiriDB (see [siridb_select_t](#siridb_select_t).
`siridb_series_t` can also be used to insert new data into SiriDB.

*Public members*
- `siridb_series_tp siridb_series_t.tp`: Series type.
  - `SIRIDB_SERIES_TP_INT64`
  - `SIRIDB_SERIES_TP_REAL`
  - `SIRIDB_SERIES_TP_STR` *(not implemented in siridb-server at this moment)*
- `char * siridb_series_t.name`: Name (identifier) for the series.
- `size_t siridb_series_t.n`: Number of points.
- `siridb_point_t siridb_series_t.points[]`: Array of `n` points.

#### `siridb_series_t * siridb_series_create(siridb_series_tp tp, char * name, size_t size)`
Creates and returns a pointer to a `siridb_series_t` instance. Argument `size`
defines the number of points the series can hold. If required it can be resized
using `siridb_series_resize()`.

Example creating an integer series with some sample data:
```c
siridb_series_t * series = siridb_series_create(
    SIRIDB_SERIES_TP_INT64,     /* type integer */
    "just-an-example",          /* name for the series */
    5);                         /* number of points */

for (size_t i = 0; i < series->n; i++)
{
    point = series->points + i;
    /* set time-stamps for the last n seconds */
    point->ts = (uint64_t) time(NULL) - series->n + i;
    point->via.int64 = (int64_t) i;
}
```

#### `void siridb_series_destroy(siridb_series_t * series)`
Cleanup `siridb_series_t`.

#### `int siridb_series_resize(siridb_series_t ** series, size_t n)`
Resize an existing series object to `n` number of points. Returns 0 if
successful or `ERR_MEM_ALLOC` in case or a memory allocation error.

### `siridb_point_t`
SiriDB Point type. A point represents a time-stamp and value and is always part
of a points array in a [siridb_series_t](#siridb_series_t) object.

*Public members*
- `uint64_t siridb_point_t.ts`: time-stamp
- `siridb_point_via_t siridb_point_t.via`: value. (Type is defined by the series)
  - `int64_t int64`
  - `double real`
  - `char * str`

#### `void siridb_resp_destroy(siridb_resp_t * resp)`
Cleanup `siridb_resp_t`.

### Miscellaneous functions
#### `const char * siridb_strerror(int err_code)`
Returns the error message for a given error code.

#### `const char * siridb_version(void)`
Returns the version of libsiridb.

