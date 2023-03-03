#include "typing.h"
#include <stdio.h>
#include <string.h>

void typing_get_indicator(struct typing_users *typing, char *indicator) {
    if (typing->amt == 0) {
        memcpy(indicator, "", 1);
    } else if (typing->amt == 1) {
        snprintf(indicator, 63, "%s is typing...", typing->users[0]);
    } else if (typing->amt == 2) {
        snprintf(indicator, 116, "%s & %s are typing...", typing->users[0],
                 typing->users[1]);
    } else if (typing->amt > 2) {
        memcpy(indicator, "Several people are typing...", 29);
    }
}

void typing_add_user(struct typing_users *typing, char *name, int name_len) {
    if (typing->amt <= 2) {
        memcpy(typing->users[typing->amt++], name, name_len);
    } else if (typing->amt > 2) {
        typing->amt++;
    }
}
