#include "messages.h"

int messages_init(struct messages *msgs, int size) {
	msgs->messages = malloc(sizeof(char*) * size);
	if (msgs->messages == NULL) return ALLOC;
	msgs->lengths = malloc(sizeof(int) * size);
	if (msgs->lengths == NULL) return ALLOC;
	msgs->size = size;
	msgs->len = 0;
	return NONE;
}

// copies all memory from `msg`
int append(struct messages *msgs, const char *msg, int len) {
	if (msgs->len >= msgs->size) {
		msgs->size *= 2;
		msgs->messages = realloc(msgs->messages, sizeof(char*) * msgs->size);
		if (msgs->messages == NULL) return ALLOC;
		msgs->lengths = realloc(msgs->lengths, sizeof(int) * msgs->size);
		if (msgs->lengths == NULL) return ALLOC;
	}
	msgs->messages[msgs->len] = malloc(len);
	if (msgs->messages[msgs->len] == NULL) return ALLOC;
	memcpy(msgs->messages[msgs->len], msg, len);
	msgs->lengths[msgs->len] = len;
	msgs->len++;
	return NONE;
}
