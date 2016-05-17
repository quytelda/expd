CC=gcc
CFLAGS=-Wall

BIN=expd
SRCS=server/main.c server/server.c
OBJS=$(SRCS:.c=.o)

.PHONY: clean

all: $(BIN)

$(BIN): $(OBJS)
	gcc -o $(BIN) $(CFLAGS) $(OBJS)

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm $(BIN) $(OBJS)
