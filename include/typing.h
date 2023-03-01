#pragma once
#include <stdio.h>

struct typing_users {
    char *users[3];
    int amt;
};

void get_indicator(struct typing_users *, char *indicator);
