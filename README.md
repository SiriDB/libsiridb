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
map the result to the request.

## API

### siridb_t
SiriDB Client type. Pending request are stored in a queue on this object until
a response is received or the request is cancelled.

#### siridb_t * siridb\_create(void)
Creates a new SiriDB Client instance. In case of an error NULL will be returned.

#### void siridb\_destroy(siridb\_t * siridb)
Cleanup a SiriDB Client instance. In case the queue has pending request then each
request will be cancelled. This means the requests status is set to `ERR_CANCELLED`
and the callback function will be called.

#### void siridb\_on\_pkg(siridb\_t * siridb, siridb\_pkg\_t * pkg)

```
siridb_handle_create(siridb_t * siridb, )
```


