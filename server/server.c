#include "server.h"
#include "command.h"
#include <signal.h>
#include <time.h>
#include <stdarg.h>

#define PORT 6969
#define CLI_MIN 16
#define BUFSIZE 4096

int broadcast_message(struct users *users, uint32_t sender,
					  uint32_t len, const char *msg) {
	for (uint32_t i = 0; i < users->len; i++) {
		send_response(users->pfds[i+1].fd, R_MSG,
					  users->name_lens[sender], users->names[sender], len, msg);
	}
	return NONE;
}

int broadcast_typing(struct users *users, uint32_t sender, uint32_t ev) {
	for (uint32_t i = 0; i < users->len; i++) {
		send_response(users->pfds[i+1].fd, ev,
					  users->name_lens[sender], users->names[sender]);
	}
	return NONE;
}

int broadcast_join(struct users *users, uint32_t sender) {
	for (uint32_t i = 0; i < users->len; i++) {
		if (i == sender) continue;
		send_response(users->pfds[i+1].fd, R_USER_JOIN,
					  users->name_lens[sender], users->names[sender]);
	}
	return NONE;
}

int send_event(struct users *users, uint32_t event, ...) {
	char *buf;
	int sender, num;
	va_list list;

	va_start(list, event);

	switch (event) {
	case R_MSG:
		sender = va_arg(list, int);
		num = va_arg(list, uint32_t);
		buf = va_arg(list, char*);
		broadcast_message(users, sender, num, buf);
		break;
	case R_GETNICK:
		sender = va_arg(list, int);
		send_response(users->pfds[sender].fd, R_GETNICK,
					  users->name_lens[sender], users->names[sender]);
		break;
	case R_SETNICK:
		sender = va_arg(list, int);
		num = va_arg(list, int);
		send_response(users->pfds[sender].fd, R_SETNICK, sender, num);
		break;
	case R_STOP_TYPING:
	case R_START_TYPING:
		sender = va_arg(list, int);
		broadcast_typing(users, sender, event);
		break;
	case R_USER_JOIN:
		sender = va_arg(list, int);
		broadcast_join(users, sender);
		break;
	default:
		return UNKNOWN_CMD;
	}
	
	va_end(list);
	
	return NONE;
}

void del(struct users *users, int i) {
	users->pfds[i+1] = users->pfds[users->len+1];
	users->len--;
}

int init_names(char **names, size_t *lens, size_t amt) {
	for (size_t i = 0; i < amt; i++) {
		names[i] = malloc(16);
		if (names[i] == NULL) return ALLOC;
		lens[i] = snprintf(names[i], 16, "User %d", (int)i);
	}
	return NONE;
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
	
	users->len = 0;
	users->size = CLI_MIN;
	if (init_names(users->names, users->name_lens, 16) != NONE)
		return ALLOC;
	return NONE;
}

int main(void) {
	char *buf;
	size_t buflen = 65536;
	int conn, pres, fd;
	struct sockaddr_in serv = {0};
	struct users users;
	struct command in;

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
		CHECK(pres, poll(users.pfds, users.len+1, -1));
		if (pres == -1) exit(1);

		if (users.pfds[0].revents & POLLIN) {
			CHECK(conn, accept(fd, NULL, NULL));
			if (conn != -1) {
				users.pfds[users.len+1].fd = conn;
				users.pfds[users.len+1].events = POLLIN | POLLPRI;
				//send_event(&users, R_USER_JOIN, users.len);
				users.len++;
			}
		}

		for (uint32_t i = 0; i < users.len; i++) {
			if ((users.pfds[i+1].revents & POLLIN) == 0) {
				if (users.pfds[i+1].revents == 0) continue;
				del(&users, i);
			}

			if (receive_command(users.pfds[i+1].fd, &in) != NONE) {
				fprintf(stderr, "alloc error");
				return 1;
			}

		    switch (in.type) {
			case C_STOP_TYPING:
				send_event(&users, R_STOP_TYPING, i);
				break;
			case C_START_TYPING:
				send_event(&users, R_START_TYPING, i);
				break;
			case C_SETNICK:
				free(users.names[i]);
				users.names[i] = in.setnick.value;
				users.name_lens[i] = in.setnick.len;
				send_event(&users, R_SETNICK, i, 1);
				break;
			case C_MSG:
				send_event(&users, R_MSG, i, in.msg.len, in.msg.value);
				free(in.msg.value);
				break;
			}
		}
	}
	
	close(fd);
}
