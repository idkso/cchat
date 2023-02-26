#pragma once
#include "common.h"

struct messages {
	char **messages;
	int *lengths;
	int len;
	int size;
};

int messages_init(struct messages *msgs, int size);
int append(struct messages *msgs, const char *msg, int len);