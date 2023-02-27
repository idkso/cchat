#include "server.h"
#include <signal.h>
#include <time.h>

#define PORT 6969
#define CLI_MIN 16
#define BUFSIZE 4096

void broadcast(struct pollfd *pfds, int nclis,
			   const char *name, int namelen,
			   const char *msg, int len) {
	struct timespec t1 = {0}, t2 = {0};
	for (int i = 1; i < nclis; i++) {
		CHEXIT(dprintf(pfds[i].fd, "%.*s: %.*s", namelen, name, len, msg));
	}
}

void del(struct pollfd *pfds, int nclis, int i) {
	pfds[i] = pfds[nclis-1];
}

void init_names(char **names, size_t *lens, size_t amt) {
	for (size_t i = 0; i < amt; i++) {
		names[i] = malloc(16);
		lens[i] = 16;
		snprintf(names[i], 16, "User %d", (int)i);
	}
}

int users_init(struct users *users) {
	users->pfds = malloc(sizeof(struct pollfd) * CLI_MIN+1);
	if (users->pfds == NULL) return ALLOC;
	
	users->names = malloc(sizeof(char*) * CLI_MIN);
	if (users->names == NULL) return ALLOC;
	
	users->name_lens = malloc(sizeof(size_t) * CLI_MIN);
	if (users->name_lens == NULL) return ALLOC;

	users->buffers = malloc(sizeof(char*) * CLI_MIN);
	if (users->buffers == NULL) return ALLOC;

	users->buf_lens = malloc(sizeof(size_t) * CLI_MIN);
	if (users->buf_lens == NULL) return ALLOC;
	
	for (int i = 0; i < CLI_MIN; i++) {
		users->buffers[i] = malloc(BUFSIZE);
		if (users->buffers[i] == NULL) return ALLOC;
	}
	
	users->len = 1;
	users->size = CLI_MIN;
	init_names(users->names, users->name_lens, 16);
	return NONE;
}

int main(void) {
	char *buf;
	size_t buflen = 65536;
	int conn, len, pres, fd;
	struct sockaddr_in serv = {0};
	struct users users;

	buf = malloc(buflen);
	if (users_init(&users) != NONE || buf == NULL) {
		fprintf(stderr, "error allocating memory\n");
		exit(1);
	}

	CHECK(fd, socket(AF_INET, SOCK_STREAM, 0));
	if (fd == -1) exit(1);
	CHEXIT(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)));

	serv.sin_family = AF_INET;
	serv.sin_addr.s_addr = htonl(INADDR_ANY);
	serv.sin_port = htons(PORT);

	CHEXIT(bind(fd, (struct sockaddr*)&serv, sizeof(serv)));

	CHEXIT(listen(fd, 8));

	users.pfds[0].fd = fd;
	users.pfds[0].events = POLLIN | POLLPRI;
	
	while (true) {
		CHECK(pres, poll(users.pfds, users.len, -1));
		if (pres == -1) exit(1);

		if (users.pfds[0].revents & POLLIN) {
			CHECK(conn, accept(fd, NULL, NULL));
			if (conn != -1) {
				broadcast(users.pfds, users.len, "server",
						  6, "someone joined", 14);
				
				users.pfds[users.len].fd = conn;
				users.pfds[users.len].events = POLLIN | POLLPRI;
				users.len++;
			}
		}

		for (size_t i = 1; i < users.len; i++) {
			if ((users.pfds[i].revents & POLLIN) == 0)
				continue;

			CHECK(len, read(users.pfds[i].fd, buf, buflen));
			if (len <= 0) {
				del(users.pfds, users.len--, i);
				continue;
			}

			broadcast(users.pfds, users.len, users.names[i],
					  users.name_lens[i], buf, len);
		}
	}
	
	close(fd);
}
