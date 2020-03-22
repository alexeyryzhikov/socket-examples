#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>

volatile sig_atomic_t is_running = 1;

void sighandler(int sig) { 
    is_running = 0;
}

void setup_sighandler() {
	struct sigaction sa;
    sa.sa_handler = sighandler;
    sa.sa_flags = 0; 
    sigemptyset(&sa.sa_mask);
 
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction() failed");
        exit(1);
    }
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction() failed");
        exit(1);
    }
}

#define MAX_QUEUE_LENGTH 10

int main() {
	setup_sighandler();

	int serversock = socket(PF_UNIX, SOCK_STREAM, 0);
	if (serversock == -1) {
		perror("socket() failed");
		exit(1);
	}

	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, "server.sock");
  	unlink(addr.sun_path);

	if (bind(serversock, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		perror("bind() failed");
		exit(1);
	};

	if (listen(serversock, MAX_QUEUE_LENGTH) == -1) {
		perror("listen() failed");
		exit(1);
	}

	printf("Server listening...\n");

	struct sockaddr clientaddr;
	socklen_t addr_size = sizeof clientaddr;

	while (is_running) {
		int sock = accept(serversock, &clientaddr, &addr_size);
		if (sock == -1) {
			perror("accept() failed");
		} else {
			printf("Client connected\n");

			char buf[512];
			size_t buf_size = 512;

			// ssize_t, not size_t !!!
			ssize_t bytes_read;
			while ((bytes_read = recv(sock, &buf, buf_size, 0)) > 0) {
				printf("%s", buf);

				// Unix sockets are two-way
				send(sock, "ack", 3, 0);
			}

			if (bytes_read == -1) {
				perror("recv() failed");
			} else { // bytes_read == 0
				printf("Client closed connection\n");
			}
			close(sock);
		}
	}

	close(serversock);

	return 0;
}