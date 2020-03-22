# socket-examples
Sample server and clients for C Socket library:
- TCP sockets
- UDP sockets
- Unix Domain Sockets
- fork, select and libevent implementations to handle multiple clients

**Note** These examples are for educational and demonstrational purposes and are not production ready.

## Building and running
Use the included Makefile to build server and client binaries: 

```
# Build sample TCP server and client
make tcp
# Start server
./server &
# Start client
./client 
```

