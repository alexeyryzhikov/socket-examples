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

#define MAX_QUEUE_LENGTH 10

void enable_reuse(int sockfd) {
	int enable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
    	perror("socket re-use failed");
	}
	#ifdef SO_REUSEPORT
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0) {
    	perror("socket re-use failed");
	}
	#endif
}

unsigned short get_port(struct sockaddr * addr) {
	struct sockaddr_in *addr_in = (struct sockaddr_in *) &addr;
	return addr_in->sin_port;
}

void handle_connection(int sock) {
	char buf[512];
	size_t buf_size = 512;

	// ssize_t, not size_t !!!
	ssize_t bytes_read;
	while ((bytes_read = recv(sock, &buf, buf_size, 0)) > 0) {
		printf("%s", buf);
	}

	if (bytes_read == -1) {
		perror("recv() failed");	
	} else { // bytes_read == 0
		printf("Client closed connection\n");
	}
	close(sock);
	exit(0);
}

int main() {
	setup_sighandler();

	struct addrinfo hints;
	struct addrinfo *servinfo; // список адресов

	memset(&hints, 0, sizeof hints); 
	hints.ai_family = AF_INET; 
	hints.ai_socktype = SOCK_STREAM; // TCP 
	hints.ai_flags = AI_PASSIVE; // система сама поставит локальный адрес

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

	enable_reuse(serversock);

	if (bind(serversock, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
		perror("bind() failed");
		exit(1);
	};

	if (listen(serversock, MAX_QUEUE_LENGTH) == -1) {
		perror("listen() failed");
		exit(1);
	}

	freeaddrinfo(servinfo);

	printf("Listening on 9999\n");

	struct sockaddr clientaddr;
	socklen_t addr_size = sizeof clientaddr;

	while (is_running) {
		int sock = accept(serversock, &clientaddr, &addr_size);
		if (sock == -1) {
			perror("accept() failed");
		} else {
			char clientaddr_str[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &clientaddr, clientaddr_str, INET_ADDRSTRLEN);
			printf("Client connected: %s:%u\n", clientaddr_str, get_port(&clientaddr));

			pid_t pid = fork();
			if (pid == -1) {
				perror("fork() failed");
			} else if (pid == 0) {
				close(serversock); // don't need it anymore
				handle_connection(sock);
			} else {
				close(sock); // don't need it anymore				
			}
		}
	}

	close(serversock);

	return 0;
}