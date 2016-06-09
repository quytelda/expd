CC=gcc
CFLAGS=-I . -Wall -pthread

BIN=expd
SRCS=server/main.c server/expd.c exp/message.c
OBJS=$(SRCS:.c=.o)

.PHONY: clean debug

all: $(BIN)

$(BIN): $(OBJS)
	gcc -o $(BIN) $(CFLAGS) $(OBJS)

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm $(BIN) $(OBJS)
