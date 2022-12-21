.PHONY: all clean

all: test

test: avl.c avl.h test.c
	cc -o $@ avl.c test.c
clean:
	rm test
