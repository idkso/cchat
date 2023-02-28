#include "messages.h"

int messages_init(struct messages *msgs, int size) {
	msgs->names = malloc(sizeof(char*) * size);
	if (msgs->names == NULL) return ALLOC;
	msgs->name_lens = malloc(sizeof(int) * size);
	if (msgs->name_lens == NULL) return ALLOC;
	msgs->msgs = malloc(sizeof(char*) * size);
	if (msgs->msgs == NULL) return ALLOC;
	msgs->msg_lens = malloc(sizeof(int) * size);
	if (msgs->msg_lens == NULL) return ALLOC;
	msgs->size = size;
	msgs->len = 0;
	return NONE;
}

// copies all memory from `msg`
int append(struct messages *msgs, const char *msg, int msg_len,
		   const char *name, int name_len) {
	if (msgs->len >= msgs->size) {
		msgs->size *= 2;
		msgs->names = realloc(msgs->names, sizeof(char*) * msgs->size);
		if (msgs->names == NULL) return ALLOC;
		msgs->name_lens = realloc(msgs->name_lens, sizeof(int) * msgs->size);
		if (msgs->name_lens == NULL) return ALLOC;
		msgs->msgs = realloc(msgs->msgs, sizeof(char*) * msgs->size);
		if (msgs->msgs == NULL) return ALLOC;
		msgs->msg_lens = realloc(msgs->msg_lens, sizeof(int) * msgs->size);
		if (msgs->msg_lens == NULL) return ALLOC;
	}
	msgs->names[msgs->len] = malloc(name_len);
	if (msgs->names[msgs->len] == NULL) return ALLOC;
	msgs->msgs[msgs->len] = malloc(msg_len);
	if (msgs->msgs[msgs->len] == NULL) return ALLOC;

	memcpy(msgs->names[msgs->len], name, name_len);
	memcpy(msgs->msgs[msgs->len], msg, msg_len);

	msgs->name_lens[msgs->len] = name_len;
	msgs->msg_lens[msgs->len] = msg_len;
	
	msgs->len++;
	return NONE;
}
