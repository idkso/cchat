#include "server.h"
#include "command.h"
#include "common.h"
#include <stdarg.h>

void send_event(struct users *users, uint32_t event, uint32_t user, ...) {
    char *buf;
    int num;
    va_list list;

    va_start(list, user);

    switch (event) {
    case R_MSG:
        num = va_arg(list, uint32_t);
        buf = va_arg(list, char *);
        for (uint32_t i = 0; i < users->len; i++) {
            if (send_response(users->pfds[i].fd, R_MSG, users->name_lens[user],
                              users->names[user], num, buf) == DISCONNECT) {
                users_del(users, i);
            }
        }
        break;
    case R_START_TYPING:
    case R_STOP_TYPING:
        for (uint32_t i = 0; i < users->len; i++) {
            if (i == user)
                continue;
            if (send_response(users->pfds[i].fd, event, users->name_lens[user],
                              users->names[user]) == DISCONNECT) {
                users_del(users, i);
            }
        }
        break;
    case R_GETNICK:
        if (send_response(users->pfds[user].fd, R_GETNICK,
                          users->name_lens[user],
                          users->names[user]) == DISCONNECT) {
            users_del(users, user);
        }
        break;
    case R_SETNICK:
        num = va_arg(list, int);
        if (send_response(users->pfds[user].fd, R_SETNICK, num) == DISCONNECT) {
            users_del(users, user);
        }
        break;
    default:
        err("unknown command");
    }

    va_end(list);
}

int server_init(uint16_t port) {
    int fd;
    struct sockaddr_in serv = {0};

    CHECK(fd, socket(AF_INET, SOCK_STREAM, 0));
    if (fd == -1)
        exit(1);
    CHEXIT(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)));

    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = htonl(INADDR_ANY);
    serv.sin_port = htons(port);

    CHEXIT(bind(fd, (struct sockaddr *)&serv, sizeof(serv)));
    CHEXIT(listen(fd, 8));
    return fd;
}

void server_listen(int fd, int backlog) { CHEXIT(listen(fd, backlog)); }

void server_poll(struct users *users) {
    int conn;

    CHEXIT(poll(users->pfds - 1, users->len + 1, -1));

    if ((users->pfds[-1].revents & POLLIN) == 0)
        return;

    CHECK(conn, accept(users->pfds[-1].fd, NULL, NULL));
    if (conn == -1) {
        return;
    }

    printf("adding user %lu\n", users->len);
    users->pfds[users->len].fd = conn;
    users->pfds[users->len].events = POLLIN | POLLPRI;
    users->len++;
}

void server_process_events(struct users *users) {
    int ret;
    struct command in;

    for (uint32_t i = 0; i < users->len; i++) {
        if ((users->pfds[i].revents & POLLIN) == 0) {
            if (users->pfds[i].revents & POLLHUP)
                users_del(users, i);
            continue;
        }

        if ((ret = receive_command(users->pfds[i].fd, &in)) != NONE) {
            if (ret == DISCONNECT) {
                users_del(users, i);
                continue;
            } else {
                err("error allocating memory");
                exit(1);
            }
        }

        switch (in.type) {
        case C_STOP_TYPING:
            send_event(users, R_STOP_TYPING, i);
            break;
        case C_START_TYPING:
            send_event(users, R_START_TYPING, i);
            break;
        case C_SETNICK:
            free(users->names[i]);
            users->names[i] = in.setnick.value;
            users->name_lens[i] = in.setnick.len;
            send_event(users, R_SETNICK, i, 1);
            break;
        case C_GETNICK:
            send_event(users, R_GETNICK, i, users->name_lens[i],
                       users->names[i]);
            break;
        case C_MSG:
            send_event(users, R_MSG, i, in.msg.len, in.msg.value);
            free(in.msg.value);
            break;
        }
    }
}
