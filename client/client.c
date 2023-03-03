#include "command.h"
#include "common.h"
#include "input.h"
#include "messages.h"
#include "typing.h"
#include <fcntl.h>
#include <netdb.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

struct winsize size;

struct termios raw;
struct termios orig;
struct pollfd pfds[2];
struct messages msgs;
struct typing_users typing;
struct response in;
struct input inp;

void uncook(void) {
    tcgetattr(STDIN_FILENO, &orig);
    raw = orig;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_iflag &= ~(ICRNL | IGNBRK);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

void cook(void) { tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig); }

void init(const char *hostname, const char *port, int *fd,
          struct pollfd pfds[2]) {
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
        if (*fd == -1)
            continue;

        CHECK(ret, connect(*fd, p->ai_addr, p->ai_addrlen));
        if (ret == -1)
            continue;

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

void print_screen(int tty, int screen_size, char *username, char *indicator) {

    dprintf(tty, "\x1b[H\x1b[J");
    for (int x = 0; x < msgs.len; x++) {
        dprintf(tty, "%.*s\n", msgs.msg_lens[x], msgs.msgs[x]);
    }
    typing_get_indicator(&typing, indicator);
    dprintf(tty, "\x1b[%d;0H%s\n[%s] >> %.*s", screen_size, indicator, username,
            inp.msg_len, inp.msg);
}

int main(int argc, char *argv[]) {
    uncook();
    ioctl(0, TIOCGWINSZ, &size);

    char *port;
    char c;
    int tty = open("/dev/tty", O_WRONLY);
    int len, fd = 0;
    int screen_size = size.ws_row;
    char username[50] = "username";
    char indicator[50];

    if (argc < 2) {
        fprintf(stderr, "usage: %s <hostname> [port]\n", argv[0]);
        return 1;
    }

    port = argc == 3 ? argv[2] : "6969";

    messages_init(&msgs, 69);
    input_init(&inp);
    init(argv[1], port, &fd, pfds);

    send_command(pfds[1].fd, C_GETNICK);
    dprintf(tty, "\x1b[%d;0H[%s] >> ", screen_size, username);

    while (true) {
        CHEXIT(poll(pfds, 2, -1));

        if (pfds[0].revents & POLLIN) { //  User types a message
            CHECK(len, read(STDIN_FILENO, &c, 1));
            if (len == -1)
                exit(1);

            input_parse(&inp, &pfds[1], c);
            print_screen(tty, screen_size, username, indicator);
        }

        if (pfds[1].revents & POLLIN) { //  Server broadcasts a new message
            receive_response(pfds[1].fd, &in);

            switch (in.r) {
            case R_GETNICK:
                memcpy(username, in.getnick.value, in.getnick.len);
                break;
            case R_MSG:
                append(&msgs, in.msg.msg, in.msg.msg_len, in.msg.name,
                       in.msg.name_len);
                break;
            case R_START_TYPING:
                typing_add_user(&typing, in.start_typing.value,
                                in.start_typing.len);
                break;
            case R_STOP_TYPING:
                typing.amt--;
                break;
            }

            print_screen(tty, screen_size, username, indicator);
        }
    }
}
