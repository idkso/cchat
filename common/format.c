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
	case MSG:
	case SETNICK:
		len = va_arg(list, uint32_t);
		string = va_arg(list, char*);
		_nputs(fd, (void*)&cmd, sizeof(uint32_t));
		_nputs(fd, (void*)&len, sizeof(uint32_t));
		_nputs(fd, string, len);
		break;
	case GETNICK:
		_nputs(fd, (void*)&cmd, sizeof(uint32_t));
		break;
	default:
		return UNKNOWN_CMD;
	}
	_putc(fd, '\0');
	
	va_end(list);
	return NONE;
}
