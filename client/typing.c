#include "typing.h"
#include <stdio.h>
#include <string.h>


void get_indicator(struct typing_users *typing, char *indicator) {
	if (typing->amt == 0) {
		indicator = "";
	} else if (typing->amt == 1) {
		snprintf(indicator, 50, "%s is typing...", typing->users[0]);
	} else if (typing->amt == 2) {
		snprintf(indicator, 50, "%s & %s are typing...", typing->users[0], typing->users[1]);
	} else if (typing->amt > 2) {
		indicator = "Several people are typing...";
	}
}