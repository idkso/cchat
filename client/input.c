#include "input.h"
#include "command.h"
#include "common.h"

void input_init(struct input *inp) {
    inp->msg_len = 0;
    inp->typing = false;
}

int input_parse(struct input *inp, struct pollfd *fd, char c) {
    if (c == 127) { //  Backspace
        if (inp->msg_len <= 0) {
            return NONE;
        } //  Dont let msg_len go negative
        inp->msg[inp->msg_len--] = '\0';
    } else if (c == 13) { //  Enter
        send_command(fd->fd, C_MSG, inp->msg_len, inp->msg);
        inp->msg[0] = '\0';
        inp->msg_len = 0;
    } else { //  Insert char
        inp->msg[inp->msg_len++] = c;
    }

    if (inp->typing == false && inp->msg_len > 0) {
        inp->typing = true;
        send_command(fd->fd, C_START_TYPING);
    } else if (inp->typing && inp->msg_len == 0) {
        inp->typing = false;
        send_command(fd->fd, C_STOP_TYPING);
    }
    return NONE;
}
