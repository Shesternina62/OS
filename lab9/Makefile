CC = gcc
TARGETS = pth sender receiver

SRC_pipe = pth.c
SRC_writer = sender.c
SRC_reader = receiver.c

all: $(TARGETS)

pipe: $(SRC_pth)
	$(CC) -o $@ $<

writer: $(SRC_sender)
	$(CC) -o $@ $<

reader: $(SRC_receiver)
	$(CC) -o $@ $<

clean:
	rm -f $(TARGETS) *.o

.PHONY: all clean
