.PHONY: all clean

CC = cc

all: test

test: lib/avl.c lib/avl.h test.c
	$(CC) -O3 -o $@ -Wall -Wextra -Wno-nullability-completeness -Werror -I./lib lib/avl.c test.c

clean:
	rm test
