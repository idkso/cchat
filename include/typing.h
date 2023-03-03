#pragma once
#include <stdio.h>

struct typing_users {
    char users[3][50];
    int amt;
};

void typing_get_indicator(struct typing_users *, char *indicator);
void typing_add_user(struct typing_users *, char *name, int name_len);
