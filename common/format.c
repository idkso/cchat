#include "common.h"

#include <stdio.h>
#include <fcntl.h>
#include <memory.h>
#include <unistd.h>

#define MIN(x, y) (((x) > (y)) ? (y) : (x))

static char buf[256];
static int size = 0;
static int place = 0;

size_t bread(int fd, char *out, int len) {
  int red, res, op = 0;

  if (place + len > size) {
    memcpy(out, buf+place, size-place);
    len -= size-place;
    op += size-place;

    red = read(fd, buf, 256);
    res = MIN(red, len);
    memcpy(out+op, buf, res);
    place = res;
    size = red;
  } else {
    memcpy(out, buf+place, len);
    place += len;
    res = len;
  }
  return res;
}

void process_message(int fd) {
	char name[16];
	char message[256];
	uint32_t len;
	bread(fd, (void*)&len, sizeof(uint32_t));
	bread(fd, name, len);
	bread(fd, (void*)&len, sizeof(uint32_t));
	bread(fd, message, len);
}
