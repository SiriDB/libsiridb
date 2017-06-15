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
- `uint8_t siridb_pkg_t.checkbit`: Checkbit. (readonly)
- `unsigned char siridb_pkg_t.data[]`: Empty or content serialized using libqpack. (readonly)


### Miscellaneous functions
#### `const char * siridb_strerror(int err_code)`
Returns the error message for a given error code.

#### `const char * siridb_version(void)`
Returns the version of libsiridb.

