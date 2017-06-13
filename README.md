# SiriDB Connector C
SiriDB Connector C is a library which can be used to communicate with SiriDB
using the C program language. This library contains usefull functions but does
not handle the connection itself. When using libuv we do have a full example.

Siridb can handle multiple queries and/or inserts on a single connection
simultanious. The order in which results are returned is not defined and as soon
as a request is finished the result will be returned. This means a client should
keep track of each request which is done using a pid. This pid is an
unsigned 16bit integer value and the client is responsable for chosing a unique
pid. SiriDB simple returns the same pid in its response so the client is able to
map the result to the request. Assigning a unique pid to a request and mapping a
response to the request is the resposibility of this library.

---------------------------------------
  * [siridb_t](#siridb-t)
    * [members](#siridb-members)
    * [api](#siridb-api)

---------------------------------------

## siridb_t
SiriDB Client type. Pending request are stored in a queue on this object until
a response is received or the request is cancelled.

### MEMBERS ###{siridb-members}
#### `void * siridb_t.data`
Space for user-defined arbitrary data. siridblib does not use this field.

### API ###{siridb-api}
#### `siridb_t * siridb_create(void)`
Creates a new SiriDB Client instance. In case of an error NULL will be returned.

#### `void siridb_destroy(siridb_t * siridb)`
Cleanup a SiriDB Client instance. In case the queue has pending request then each
request will be cancelled. This means the requests status will be set to
`ERR_CANCELLED` and the callback function will be called.

#### `void siridb_on_pkg(siridb_t * siridb, siridb_pkg_t * pkg)`
Should be called when a package is received on a SiriDB connection. This
function then takes the request from the queue and calls the callback function.
In case the request cannot be found `ERR_NOT_FOUND` is returned otherwise 0 is
returned.

#### `size_t siridb_queue_len(siridb_t * siridb)`
Returns the numbers of requests in the queue.


