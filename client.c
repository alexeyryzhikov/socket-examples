#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>

sig_atomic_t is_running = 1;

void sighandler(int sig) { 
    is_running = 0;
    if (sig == SIGPIPE) {
    	write(1, "Server closed connection\n", 25);
    }
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

    if (sigaction(SIGPIPE, &sa, NULL) == -1) {
        perror("sigaction() failed");
        exit(1);
    }
}

int main() {
	setup_sighandler();

	struct addrinfo hints;
	struct addrinfo *servinfo; 

	memset(&hints, 0, sizeof hints); 
	hints.ai_family = AF_INET; 
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; 

	int status;
	if ((status = getaddrinfo(NULL, "9999", &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}

	int sock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if (sock == -1) {
		perror("socket() failed");
		exit(1);
	}

	int enable = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, &enable, sizeof(int)) < 0) {
    	perror("setsockopt() failed");
	}

	if (connect(sock, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
		perror("connect() failed");
		exit(1);
	}

	freeaddrinfo(servinfo);
	// bind is not required if you're not concerned about the client port

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
	}
	free(line);

	close(sock);

	return 0;
}