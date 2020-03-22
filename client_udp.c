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
}

int main() {
	setup_sighandler();

	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1) {
		perror("socket() failed");
		exit(1);
	}

	struct sockaddr_in addr;
	inet_pton(AF_INET, "127.0.0.1", &(addr.sin_addr));
	addr.sin_port = htons(9999); // important!, 9999 won't work

	char* line = NULL;
	size_t len = 0;
	ssize_t read = 0;
	while (is_running) {
		read = getline(&line, &len, stdin);

		if (read != -1) {
			for (size_t i=0; i < len; ) {

				int bytes_sent = sendto(sock, line, len, 0, (struct sockaddr *)&addr, sizeof(addr)); 
				if (bytes_sent < 0) {	
					perror("send() failed");
				} else {
					i += bytes_sent;
				}
			}
		}
	}
	free(line);

	close(sock);

	return 0;
}