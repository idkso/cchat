#include "common.h"
#include "messages.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

struct winsize size;


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


void print_messages(int tty, struct messages *msgs) {
	dprintf(tty, "\x1b[H\x1b[J");
	for (int x = 0; x < msgs->len; x++) {
		dprintf(tty, "%.*s\n", msgs->lengths[x], msgs->messages[x]);
	}

}


int main(int argc, char *argv[]) {
	ioctl(0, TIOCGWINSZ, &size);

	char buf[256], *port;
	int tty = open("/dev/tty", O_WRONLY);
	int len, fd = 0; 
	int screen_size = size.ws_row;
	char username[] = "username";

	struct pollfd pfds[2];
	struct messages msgs;

	if (argc < 2) {
		fprintf(stderr, "usage: %s <hostname> [port]\n", argv[0]);
		return 1;
	}

	port = argc == 3 ? argv[2] : "6969";

	messages_init(&msgs, 69);
	init(argv[1], port, &fd, pfds);

	dprintf(tty, "\x1b[%d;0H[%s] >> ", screen_size, username);


	while (true) {
		CHEXIT(poll(pfds, 2, -1));

		if (pfds[0].revents & POLLIN) { //  User types a message
			CHECK(len, read(STDIN_FILENO, buf, 256));
			if (len == -1) exit(1);

			CHEXIT(write(fd, buf, len));
		}
		
		if (pfds[1].revents & POLLIN ) { //  Server broadcasts a new message
			CHECK(len, read(fd, buf, 256));
			if (len <= 0) exit(1);

			append(&msgs, buf, len);
			print_messages(tty, &msgs);
			dprintf(tty, "\x1b[%d;0H[%s] >> ", screen_size, username);
		}
	}
}
