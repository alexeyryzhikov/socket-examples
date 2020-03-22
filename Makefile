clean:
	rm -f server client server_udp client_udp

tcp: 
	gcc server.c -o server
	gcc client.c -o client

tcp-fork:
	gcc server_fork.c -o server
	gcc client.c -o client

libevent:
	gcc server_async_libevent.c -levent -o server

select:
	gcc server_async_select.c -o server

udp:
	gcc server_udp.c -o server_udp
	gcc client_udp.c -o client_udp

unix:
	gcc server_unix.c -o server_unix
	gcc client_unix.c -o client_unix