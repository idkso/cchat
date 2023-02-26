#pragma once
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <poll.h>

#include <sys/socket.h>
#include <netinet/in.h>

#define CHECK(out, x)													\
	do {																\
		out = (x);														\
		if (out == -1) {												\
			fprintf(stderr, "error at %s:%d@%s '%s': %s\n",				\
					__FILE__, __LINE__, __func__, #x, strerror(errno)); \
		}																\
	} while(0)

#define CHEXIT(x)								\
	do {										\
		if ((x) == -1) {												\
			fprintf(stderr, "error at %s:%d@%s '%s': %s\n",				\
					__FILE__, __LINE__, __func__, #x, strerror(errno)); \
			exit(1);													\
		}																\
	} while (0)


enum errors {
	NONE,
	ALLOC,
	UNKNOWN_CMD,
	INVALID_ARGS,
};
