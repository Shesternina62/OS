CC = gcc
TARGET = archiver
SRC = archiver.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) -o $@ $<

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean
