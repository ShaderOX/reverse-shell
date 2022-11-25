CC=clang
CFLAGS=-Wall -Wextra -g

BIN_DIR=bin
BIN=server client test 

all: $(BIN)

%: %.c
	$(CC) $(CFLAGS) -o $(BIN_DIR)/$@ $<


clean: $(BIS)
	rm -fr $(BIN_DIR)/*
