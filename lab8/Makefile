CC = gcc
CFLAGS = -Wall -pthread 

TARGETS = pth

all: $(TARGETS)

pth: pth.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS) *.o

.PHONY: all clean
