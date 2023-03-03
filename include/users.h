#pragma once
#include <stdint.h>
#include <sys/types.h>

struct users {
    struct pollfd *pfds;
    char **names;
    char **buffers;
    size_t len, size, *name_lens, *buf_lens;
};

void users_init(struct users *users, int server, int clients);
void users_del(struct users *users, int user);
