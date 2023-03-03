#pragma once
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/socket.h>

#define err(str) errf("%s", str)
#define errf(str, ...)                                                         \
    do {                                                                       \
        fprintf(stderr, "error at %s:%d@%s: ", __FILE__, __LINE__, __func__);  \
        fprintf(stderr, str "\n", __VA_ARGS__);                                \
        fflush(stderr);                                                        \
    } while (0);

#define CHECK(out, x)                                                          \
    do {                                                                       \
        out = (x);                                                             \
        if (out == -1) {                                                       \
            fprintf(stderr, "error at %s:%d@%s '%s': %s\n", __FILE__,          \
                    __LINE__, __func__, #x, strerror(errno));                  \
            fflush(stderr);                                                    \
        }                                                                      \
    } while (0)

#define CHEXIT(x)                                                              \
    do {                                                                       \
        if ((x) == -1) {                                                       \
            fprintf(stderr, "error at %s:%d@%s '%s': %s\n", __FILE__,          \
                    __LINE__, __func__, #x, strerror(errno));                  \
            fflush(stderr);                                                    \
            exit(1);                                                           \
        }                                                                      \
    } while (0)

enum errors {
    NONE,
    ALLOC,
    UNKNOWN_CMD,
    DISCONNECT,
};
