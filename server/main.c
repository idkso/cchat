#include "common.h"
#include "server.h"
#include "users.h"

int main(void) {
    int server;
    struct users users;

    server = server_init(6969);

    users_init(&users, server, 16);

    server_listen(server, 8);

    while (true) {
        server_poll(&users);

        server_process_events(&users);
    }

    close(server);
}
