#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

/*
int getaddrinfo(const char *node, // e.g. "www.example.com" or IP
const char *service, // e.g. "http" or port number
const struct addrinfo *hints,
struct addrinfo **res);
*/


void print_addresses(struct addrinfo *servinfo_list) {
	char ipstr[INET6_ADDRSTRLEN];
	struct addrinfo *info = servinfo_list;

	for (; info != NULL; info = info->ai_next) {
		void *addr;
		if (info->ai_family == AF_INET) { 
			// IPv4
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)info->ai_addr;
			addr = &(ipv4->sin_addr);
		} else { 
			// IPv6
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)info->ai_addr;
			addr = &(ipv6->sin6_addr);
		}

		inet_ntop(info->ai_family, addr, ipstr, sizeof ipstr);
		printf("%s - %s\n", info->ai_canonname, ipstr);
	}
}

void check_local_port(char *port) {
	int status;

	struct addrinfo hints;
	struct addrinfo *servinfo_list; // список адресов

	memset(&hints, 0, sizeof hints); 
	hints.ai_family = AF_UNSPEC; // IPv4 или IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP 
	hints.ai_flags = AI_PASSIVE | AI_CANONNAME; // система сама поставит локальный адрес


	if ((status = getaddrinfo(NULL, port, &hints, &servinfo_list)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}

	print_addresses(servinfo_list);

	freeaddrinfo(servinfo_list);
}

int main() {
	check_local_port("8080");
}