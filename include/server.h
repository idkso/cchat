#pragma once
#include "users.h"
#include <stdint.h>

int server_init(uint16_t port);
void server_listen(int fd, int backlog);
void server_poll(struct users *users);
void server_process_events(struct users *users);
