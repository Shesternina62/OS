CC = gcc
TARGET = myls

all: $(TARGET)

$(TARGET): myls.c
	$(CC) -o $@ $<

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean