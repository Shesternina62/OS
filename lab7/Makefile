CC = gcc
CFLAGS = -Wall -pthread 

TARGETS = sender receiver

all: $(TARGETS)

sender: sender.c
	$(CC) $(CFLAGS) -o $@ $<

receiver: receiver.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS) *.o

.PHONY: all clean
