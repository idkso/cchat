#pragma once
#include "common.h"

enum command {
	MSG, // len(u32), string(u8*)
	GETNICK, // no operands
	SETNICK, // len(u32), string(u8*)
};

int send_command(int fd, uint32_t cmd, ...);
