#include "users.h"
#include "common.h"

int init_names(char **names, size_t *lens, size_t amt) {
    for (size_t i = 0; i < amt; i++) {
        names[i] = malloc(16);
        if (names[i] == NULL)
            return ALLOC;
        lens[i] = snprintf(names[i], 16, "User %d", (int)i);
    }

    return NONE;
}

#define CHALLOC(ptr)                                                           \
    do {                                                                       \
        if (ptr == NULL) {                                                     \
            err("error allocating shit");                                      \
            exit(1);                                                           \
        }                                                                      \
    } while (0)

void users_init(struct users *users, int server, int clients) {
    users->pfds = malloc(sizeof(struct pollfd) * clients + 1);
    CHALLOC(users->pfds);

    users->names = malloc(sizeof(char *) * clients);
    CHALLOC(users->names);

    users->name_lens = malloc(sizeof(size_t) * clients);
    CHALLOC(users->name_lens);

    if (init_names(users->names, users->name_lens, clients) != NONE) {
        err("error allocating shit");
        exit(1);
    }

    users->len = 0;
    users->size = clients;
    users->pfds[0].fd = server;
    users->pfds[0].events = POLLIN | POLLPRI;
    users->pfds++;
}

void users_del(struct users *users, int user) {
	printf("deleting user %d\n", user);
	users->len--;
    users->pfds[user] = users->pfds[users->len];
}
