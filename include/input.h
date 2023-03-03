#include <stdio.h>
#include <stdlib.h>
#include "command.h"

struct input {
	char msg[2048];
	int msg_len;
	bool typing;
};

void input_init(struct input *);
int input_parse(struct input *, struct pollfd *, char c);
