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
int append(struct messages *msgs, const char *msg, int msg_len, const char *name, int name_len) {
	if (msgs->len >= msgs->size) {
		msgs->size *= 2;
		msgs->messages = realloc(msgs->messages, sizeof(char*) * msgs->size);
		if (msgs->messages == NULL) return ALLOC;
		msgs->lengths = realloc(msgs->lengths, sizeof(int) * msgs->size);
		if (msgs->lengths == NULL) return ALLOC;
	}
	msgs->messages[msgs->len] = malloc(msg_len + name_len);
	if (msgs->messages[msgs->len] == NULL) return ALLOC;
	memcpy(msgs->messages[msgs->len], name, name_len);
	memcpy(msgs->messages[msgs->len], msg, msg_len);
	msgs->lengths[msgs->len] = msg_len + name_len;
	msgs->len++;
	return NONE;
}
