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
		printf("  %s\n", ipstr);
	}
}

void check_local_port() {
	int status;

	struct addrinfo hints;
	struct addrinfo *servinfo_list; // will point to the results

	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_UNSPEC; // don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
	hints.ai_flags = AI_PASSIVE; // fill in my IP for me

	if ((status = getaddrinfo(NULL, "8080", &hints, &servinfo_list)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}

	freeaddrinfo(servinfo_list);
}

void check_remote_address(const char *node) {
	int status;

	struct addrinfo hints;
	struct addrinfo *servinfo_list; // will point to the results

	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_UNSPEC; // don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets - если не указывать, то так же вернет отдельную запись для UDP
	
	if ((status = getaddrinfo(node, NULL, &hints, &servinfo_list)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}

	printf("%s:\n", node);
	print_addresses(servinfo_list);

	freeaddrinfo(servinfo_list);
}


int main() {
	check_remote_address("google.com");
	check_remote_address("bsu.by");
}