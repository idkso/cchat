CLIENT_SRC_ROOT := client
CLIENT_SRC := $(shell fd -e c . $(CLIENT_SRC_ROOT))
SERVER_SRC_ROOT := server
SERVER_SRC := $(shell fd -e c . $(SERVER_SRC_ROOT))
COMMON_SRC_ROOT := common
COMMON_SRC := $(shell fd -e c . $(COMMON_SRC_ROOT))
HEADER_ROOT := include
HEADERS := $(shell fd -e h . $(HEADER_ROOT))
CLIENT_OBJ := $(CLIENT_SRC:%.c=%.o)
SERVER_OBJ := $(SERVER_SRC:%.c=%.o)
COMMON_OBJ := $(COMMON_SRC:%.c=%.o)
WARNINGS := -Wall -Wextra -Wpedantic -Wsuggest-attribute=pure -Wsuggest-attribute=noreturn -Wsuggest-attribute=cold -Walloca -Wduplicated-branches -Wduplicated-cond -Wfloat-equal -Wlarger-than=4KiB -Wpointer-arith
CLIENT_OUT ?= cli
SERVER_OUT ?= serv
CFLAGS ?= -std=c11 -pipe
INCLUDE := -Iinclude
LIB :=

all: $(CLIENT_OUT) $(SERVER_OUT) compile_flags.txt

.DEFAULT_GOAL = debug

debug: CC = gcc
debug: CFLAGS += -Og -ggdb3
debug: all

analyze: CC = gcc
analyze: CFLAGS += -fanalyzer
analyze: debug

release: CC = clang
release: CFLAGS += -O2 -flto=thin
release: all

format: $(CLIENT_SRC) $(SERVER_SRC) $(COMMON_SRC) $(HEADERS)
	clang-format -i $^ 

debugger: debug
	gdb $(OUT)

$(CLIENT_OUT): $(CLIENT_OBJ) $(COMMON_OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIB)

$(SERVER_OUT): $(SERVER_OBJ) $(COMMON_OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIB)

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $(INCLUDE) $(WARNINGS) $(CFLAGS) $<

compile_flags.txt: Makefile
	rm -f compile_flags.txt
	for flag in $(WARNINGS) $(CFLAGS) $(INCLUDE); do \
		echo $$flag >> $@; \
	done

clean: $(CLIENT_OUT) $(SERVER_OUT) $(CLIENT_OBJ) $(SERVER_OBJ) compile_flags.txt
	rm -rf $^
