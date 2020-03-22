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
}

int main() {
	setup_sighandler();

	int sock = socket(PF_UNIX, SOCK_STREAM, 0);
	if (sock == -1) {
		perror("socket() failed");
		exit(1);
	}

	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, "server.sock");

	int enable = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, &enable, sizeof(int)) < 0) {
    	perror("setsockopt() failed");
	}

	if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		perror("connect() failed");
		exit(1);
	}

	char* line = NULL;
	size_t len = 0;
	ssize_t read = 0;
	while (is_running) {
		read = getline(&line, &len, stdin);

		if (read != -1) {
			for (size_t i=0; i < len; ) {
				int bytes_sent = send(sock, line, len, 0); 
				if (bytes_sent > 0) {
					i += bytes_sent;
				} else if (bytes_sent == 0) {
					printf("server closed connection\n");
					is_running = 0;
					break;
				} else {
					perror("send() failed");
					is_running = 0;
					break;
				}
			}
		} else {
			perror("read() failed");
		}

		// Communication is two-way
		char buf[512];
		size_t buf_size = 512;
		ssize_t bytes_read = recv(sock, &buf, buf_size, 0);
		if (bytes_read > 0) {
			printf("Server ack\n");
		} else if (bytes_read == 0) {
			printf("Server closed connection\n");
		} else {
			perror("recv() failed");
		}
	}
	free(line);

	close(sock);

	return 0;
}