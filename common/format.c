#include "command.h"

#include <fcntl.h>
#include <memory.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#define MIN(x, y) (((x) > (y)) ? (y) : (x))

static char buf[256];
static int len = 0;

static int _putc(int fd, int c) {
    int ret;
    if (len < 255) {
        buf[len++] = c;
    } else {
        buf[len++] = c;
        CHECK(ret, write(fd, buf, len));
        if (ret == 0)
            return DISCONNECT;
        len = 0;
    }
    return NONE;
}

static int _flush(int fd) {
    int ret;
    CHECK(ret, write(fd, buf, len));
    if (ret == 0)
        return DISCONNECT;
    len = 0;
    return NONE;
}

static int _nputs(int fd, char *s, int len) {
    for (int i = 0; i < len; i++) {
        if (_putc(fd, s[i]))
            return DISCONNECT;
    }
    return NONE;
}

int send_command(int fd, uint32_t cmd, ...) {
    uint32_t len;
    char *string;
    va_list list;

    memset(buf, 0, 256);

    va_start(list, cmd);

    switch (cmd) {
    case C_MSG:
    case C_SETNICK:
        len = va_arg(list, uint32_t);
        string = va_arg(list, char *);
        _nputs(fd, (void *)&cmd, sizeof(uint32_t));
        _nputs(fd, (void *)&len, sizeof(uint32_t));
        _nputs(fd, string, len);
        break;
    case C_GETNICK:
    case C_START_TYPING:
    case C_STOP_TYPING:
        _nputs(fd, (void *)&cmd, sizeof(uint32_t));
        break;
    default:
        return UNKNOWN_CMD;
    }
    _flush(fd);

    va_end(list);
    return NONE;
}

// allocates memory in `buf`
int read_string(int fd, uint32_t *len, char **buf) {
    int ret;
    CHECK(ret, read(fd, (void *)len, sizeof(uint32_t)));
    if (ret == 0)
        return DISCONNECT;

    *buf = malloc(*len);
    if (*buf == NULL)
        return ALLOC;

    CHECK(ret, read(fd, *buf, *len));
    if (ret == 0)
        return DISCONNECT;
    return NONE;
}

// returns allocated buffer in any pointer
int receive_response(int fd, struct response *out) {
    uint32_t op;
    char *buf;
    int ret;

    CHECK(ret, read(fd, (void *)&op, sizeof(uint32_t)));
    if (ret == 0)
        return DISCONNECT;

    switch (op) {
    case R_MSG:
        out->r = R_MSG;
        if ((ret = read_string(fd, &op, &buf)) != NONE)
            return ret;
        out->msg.name_len = op;
        out->msg.name = buf;

        if ((ret = read_string(fd, &op, &buf)) != NONE)
            return ret;
        out->msg.msg_len = op;
        out->msg.msg = buf;
        break;
    case R_SETNICK:
        CHECK(ret, read(fd, (void *)&op, sizeof(uint32_t)));
        if (ret == 0)
            return DISCONNECT;
        out->r = R_SETNICK;
        out->ok = op;
        break;
    case R_GETNICK:
    case R_START_TYPING:
    case R_STOP_TYPING:
    case R_USER_JOIN:
        out->r = op;
        if ((ret = read_string(fd, &op, &buf)) != NONE)
            return ret;
        out->stop_typing.len = op;
        out->stop_typing.value = buf;
        break;
    }

    return NONE;
}

int send_response(int fd, uint32_t cmd, ...) {
    uint32_t len;
    char *string;
    va_list list;

    memset(buf, 0, 256);

    va_start(list, cmd);

    switch (cmd) {
    case R_MSG:
        len = va_arg(list, uint32_t);
        string = va_arg(list, char *);
        if (_nputs(fd, (void *)&cmd, sizeof(uint32_t)))
            return DISCONNECT;
        if (_nputs(fd, (void *)&len, sizeof(uint32_t)))
            return DISCONNECT;
        if (_nputs(fd, string, len))
            return DISCONNECT;

        len = va_arg(list, uint32_t);
        string = va_arg(list, char *);
        if (_nputs(fd, (void *)&len, sizeof(uint32_t)))
            return DISCONNECT;
        if (_nputs(fd, string, len))
            return DISCONNECT;
        break;
    case R_GETNICK:
    case R_START_TYPING:
    case R_STOP_TYPING:
        len = va_arg(list, uint32_t);
        string = va_arg(list, char *);
        if (_nputs(fd, (void *)&cmd, sizeof(uint32_t)))
            return DISCONNECT;
        if (_nputs(fd, (void *)&len, sizeof(uint32_t)))
            return DISCONNECT;
        if (_nputs(fd, string, len))
            return DISCONNECT;
        break;
    case R_SETNICK:
        len = va_arg(list, int);
        if (_nputs(fd, (void *)&cmd, sizeof(uint32_t)))
            return DISCONNECT;
        if (_nputs(fd, (void *)&len, sizeof(int)))
            return DISCONNECT;
        break;
    default:
        return UNKNOWN_CMD;
    }
    _flush(fd);

    va_end(list);
    return NONE;
}

int receive_command(int fd, struct command *out) {
    uint32_t op;
    char *buf;
    int len, ret;

    CHECK(len, read(fd, (void *)&op, sizeof(uint32_t)));
    if (len == 0)
        return DISCONNECT;

    out->type = op;

    switch (op) {
    case C_MSG:
    case C_SETNICK:
        if ((ret = read_string(fd, &op, &buf)) != NONE)
            return ret;
        out->setnick.len = op;
        out->setnick.value = buf;
        break;
    case C_GETNICK:
    case C_START_TYPING:
    case C_STOP_TYPING:
        break;
    }

    return NONE;
}
