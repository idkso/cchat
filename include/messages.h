#pragma once
#include "common.h"

struct messages {
    char **names;
    char **msgs;
    int *msg_lens;
    int *name_lens;
    int len;
    int size;
};

int messages_init(struct messages *msgs, int size);
int append(struct messages *msgs, const char *msg, int msg_len,
           const char *name, int name_len);
