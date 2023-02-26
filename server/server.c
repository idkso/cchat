#include "common.h"

#define PORT 6969
#define CLI_MAX 16

void broadcast(struct pollfd *pfds, int nclis,
			   const char *msg, int len) {
	for (int i = 1; i < nclis; i++) {
		CHEXIT(write(pfds[i].fd, msg, len));
	}
}

void del(struct pollfd *pfds, int nclis, int i) {
	pfds[i] = pfds[nclis-1];
	pfds[nclis-1] = (struct pollfd){0};
}

int main(void) {
	char buf[256];
	int conn, len, pres, fd, nclis = 1;
	struct sockaddr_in serv = {0};
	struct pollfd pfds[CLI_MAX+1] = {{0}};

	CHECK(fd, socket(AF_INET, SOCK_STREAM, 0));
	if (fd == -1) exit(1);

	serv.sin_family = AF_INET;
	serv.sin_addr.s_addr = htonl(INADDR_ANY);
	serv.sin_port = htons(PORT);

	CHEXIT(bind(fd, (struct sockaddr*)&serv, sizeof(serv)));

	CHEXIT(listen(fd, 8));

	pfds[0].fd = fd;
	pfds[0].events = POLLIN | POLLPRI;
	
	while (true) {
		CHECK(pres, poll(pfds, nclis, -1));
		if (pres == -1) exit(1);

		if (pfds[0].revents & POLLIN) {
			CHECK(conn, accept(fd, NULL, NULL));
			if (conn != -1) {
				broadcast(pfds, nclis, "someone joined", 14);
				
				pfds[nclis].fd = conn;
				pfds[nclis].events = POLLIN | POLLPRI;
				nclis++;
			}
		}

		for (int i = 1; i < nclis; i++) {
			if ((pfds[i].revents & POLLIN) == 0)
				continue;

			CHECK(len, read(pfds[i].fd, buf, 256));
			if (len <= 0) {
				del(pfds, nclis--, i);
				continue;
			}

			broadcast(pfds, nclis, buf, len);
		}
	}
	
	close(fd);
}
