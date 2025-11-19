CC = gcc
TARGETS = pipe writer reader

SRC_pipe = pipe.c
SRC_writer = writer.c
SRC_reader = reader.c

all: $(TARGETS)

pipe: $(SRC_pipe)
	$(CC) -o $@ $<

writer: $(SRC_writer)
	$(CC) -o $@ $<

reader: $(SRC_reader)
	$(CC) -o $@ $<

clean:
	rm -f $(TARGETS) *.o myfifo

.PHONY: all clean
