CC = gcc
CFLAGS = -Wall -pthread

TARGET = lab11

all: $(TARGET)

$(TARGET): lab11.c
	$(CC) $(CFLAGS) -o $(TARGET) lab11.c

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean
