# SiriDB Connector C
Work in progress for creating a C driver for communication with SiriDB...


## API

### siridb_t
SiriDB Client type. Pending handlers are stored by PID on an instance of this
object. Implementation specific requirements like a buffer for receiving data
should be bound to the public data property.

#### siridb_t * siridb\_create()
Creates a new SiriDB Client instance.

#### void siridb\_destroy(siridb_t * siridb)

```
siridb_handle_create(siridb_t * siridb, )
```


