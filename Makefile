.PHONY: all clean test

all: tests

clean:
	rm tests

test: tests
	./tests

tests: lib/src/avl.c lib/src/test.c lib/include/avl.h
	gcc -I lib/include/ -Werror -Wall -Wextra -o tests lib/src/avl.c lib/src/test.c
