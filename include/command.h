#pragma once
#include "common.h"

enum {
	C_MSG, // len(u32), string(u8*)
	C_GETNICK, // no operands
	C_SETNICK, // len(u32), name(u8*)
	C_START_TYPING, // no operands
	C_STOP_TYPING, // no operands
};

enum {
	R_MSG, // len(u32), name(u8*), len(u32), string(u8*)
	R_GETNICK, // len(u32), name(u8*)
	R_SETNICK, // OK(u32)
	R_START_TYPING, // len(u32), name(u8*)
	R_STOP_TYPING, // len(u32), name(u8*)
};

struct msg {
	uint32_t name_len;
	char *name;
	uint32_t msg_len;
	char *msg;
};

struct name {
	uint32_t name_len;
	char *name;
};

struct response {
	uint32_t r;
	union {
		struct msg msg;
		struct name getnick;
		struct name start_typing;
		struct name stop_typing;
		int ok; // 0 if not ok, 1 if ok
	};
};

int send_command(int fd, uint32_t cmd, ...);

