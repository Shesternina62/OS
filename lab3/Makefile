CC = gcc
TARGET = myfork

all: $(TARGET)

$(TARGET): myfork.c
	$(CC) -o $@ $<

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean

