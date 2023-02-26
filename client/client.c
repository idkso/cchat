#define _POSIX_C_SOURCE 200112L
#include "common.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

void init(const char *hostname, const char *port,
		  int *fd, struct pollfd pfds[2]) {
	int ret;
	struct addrinfo hints = {0}, *servinfo, *p;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if ((ret = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
		exit(1);
	}

	for (p = servinfo; p != NULL; p = p->ai_next) {
		CHECK(*fd, socket(p->ai_family, p->ai_socktype, p->ai_protocol));
		if (*fd == -1) continue;

		CHECK(ret, connect(*fd, p->ai_addr, p->ai_addrlen));
		if (ret == -1) continue;

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "failed to connect\n");
		exit(1);
	}

	freeaddrinfo(servinfo);

	pfds[0].fd = STDIN_FILENO;
	pfds[0].events = POLLIN;
	pfds[1].fd = *fd;
	pfds[1].events = POLLIN | POLLPRI;
}

int main(int argc, char *argv[]) {
	char buf[256], *port;
	int len, fd = 0;
	struct pollfd pfds[2];

	if (argc < 2) {
		fprintf(stderr, "usage: %s <hostname> [port]\n", argv[0]);
		return 1;
	}

	port = argc == 3 ? argv[2] : "6969";

	init(argv[1], port, &fd, pfds);

	while (true) {
		CHEXIT(poll(pfds, 2, -1));

		if (pfds[0].revents & POLLIN) {
			CHECK(len, read(STDIN_FILENO, buf, 256));
			if (len == -1) exit(1);

			CHEXIT(write(fd, buf, len));
		}
		
		if (pfds[1].revents & POLLIN ) {
			CHECK(len, read(fd, buf, 256));
			if (len == -1) exit(1);

			printf("[SERVER]: %.*s\n", len, buf);
		}
	}
	
}
