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

unsigned short get_port(struct sockaddr * addr) {
	struct sockaddr_in *addr_in = (struct sockaddr_in *) &addr;
	return addr_in->sin_port;
}

int main() {
	setup_sighandler();

	struct addrinfo hints;
	struct addrinfo *servinfo;

	memset(&hints, 0, sizeof hints); 
	hints.ai_family = AF_INET; 
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; 

	int status;
	if ((status = getaddrinfo(NULL, "9999", &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo() error: %s\n", gai_strerror(status));
		exit(1);
	}

	int serversock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if (serversock == -1) {
		perror("socket() failed");
		exit(1);
	}

	if (bind(serversock, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
		perror("bind() failed");
		exit(1);
	};

	freeaddrinfo(servinfo);

	printf("Listening on 9999\n");

	struct sockaddr clientaddr;
	socklen_t addr_size = sizeof clientaddr;
	char clientaddr_str[INET_ADDRSTRLEN];

	char buf[512];
	size_t buf_size = 512;

	while (is_running) {
		// ssize_t, not size_t !!!
		ssize_t bytes_read;
		while ((bytes_read = recvfrom(serversock, &buf, buf_size, 0, &clientaddr, &addr_size)) > 0) {
			inet_ntop(AF_INET, &clientaddr, clientaddr_str, INET_ADDRSTRLEN);
			printf("message from %s:%u\n", clientaddr_str, get_port(&clientaddr));
			printf("%s", buf);
		}

		if (bytes_read == -1) {
			perror("recvfrom() failed");
		} 
	}

	close(serversock);

	return 0;
}