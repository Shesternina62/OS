CC = gcc

TARGETS = mycat mygrep

all: $(TARGETS)

mycat: mycat.c
	$(CC)  -o $@ $<

mygrep: mygrep.c
	$(CC)  -o $@ $<

clean:
	rm -f $(TARGETS) *.o

.PHONY: all clean
