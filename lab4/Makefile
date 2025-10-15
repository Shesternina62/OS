CC = gcc
TARGET = mychmod

all: $(TARGET)

$(TARGET): mychmod.c
	$(CC) -o $@ $<

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean
