#pragma once
#include "common.h"

struct users {
    struct pollfd *pfds;
    char **names;
    char **buffers;
    size_t len, size, *name_lens, *buf_lens;
};
