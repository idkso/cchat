#include "command.h"

#include <stdio.h>
#include <fcntl.h>
#include <memory.h>
#include <unistd.h>
#include <stdarg.h>

#define MIN(x, y) (((x) > (y)) ? (y) : (x))

static char buf[256];
static int len = 0;

static void _putc(int fd, int c) {
    if (len < 256 && c != '\0') {
        buf[len++] = c;
    } else {
        if (len < 255) {
            buf[len++] = c;
            write(fd, buf, len);
        } else {
			buf[len++] = c;
            write(fd, buf, len);
        }
        len = 0;
    }
}

static void _nputs(int fd, char *s, int len) {
    for (int i = 0; i < len; i++)
        _putc(fd, s[i]);
}

int send_command(int fd, uint32_t cmd, ...) {
	uint32_t len;
	char *string;
	va_list list;

	va_start(list, cmd);

	switch (cmd) {
	case C_MSG:
	case C_SETNICK:
		len = va_arg(list, uint32_t);
		string = va_arg(list, char*);
		_nputs(fd, (void*)&cmd, sizeof(uint32_t));
		_nputs(fd, (void*)&len, sizeof(uint32_t));
		_nputs(fd, string, len);
		break;
	case C_GETNICK:
	case C_START_TYPING:
	case C_STOP_TYPING:
		_nputs(fd, (void*)&cmd, sizeof(uint32_t));
		break;
	default:
		return UNKNOWN_CMD;
	}
	_putc(fd, '\0');
	
	va_end(list);
	return NONE;
}

// allocates memory in `buf`
int read_string(int fd, uint32_t *len, char **buf) {
	CHEXIT(read(fd, (void*)len, sizeof(uint32_t)));
		
	*buf = malloc(*len);
	if (*buf == NULL) return ALLOC;
	
	CHEXIT(read(fd, *buf, *len));
	return NONE;
}

// returns allocated buffer in any pointer
int receive(int fd, struct response *out) {
	uint32_t op;
	char *buf;
	int len;
	
	CHECK(len, read(fd, (void*)&op, sizeof(uint32_t)));
	if (len == -1) exit(1);

	switch (op) {
	case R_MSG:
		out->r = R_MSG;
		if (read_string(fd, &op, &buf) != NONE) return ALLOC;
		out->msg.name_len = op;
		out->msg.name = buf;

		if (read_string(fd, &op, &buf) != NONE) return ALLOC;
		out->msg.msg_len = op;
		out->msg.msg = buf;
		break;
	case R_SETNICK:
		read(fd, (void*)&op, sizeof(uint32_t));
		out->r = R_SETNICK;
		out->ok = op;
		break;
	case R_GETNICK:
		out->r = R_GETNICK;
		if (read_string(fd, &op, &buf) != NONE) return ALLOC;
		out->getnick.name_len = op;
		out->getnick.name = buf;
		break;
	case R_START_TYPING:
		out->r = R_START_TYPING;
		if (read_string(fd, &op, &buf) != NONE) return ALLOC;
		out->start_typing.name_len = op;
		out->start_typing.name = buf;
		break;
	case R_STOP_TYPING:
		out->r = R_STOP_TYPING;
		if (read_string(fd, &op, &buf) != NONE) return ALLOC;
		out->stop_typing.name_len = op;
		out->stop_typing.name = buf;
		break;
	}

	return NONE;
}
