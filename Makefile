.PHONY: all clean

.PRECIOUS: obj/%.o

all: out/test out/database_explorer

clean:
	rm -r out obj

out/%: obj/%.o obj/avl.o
	[ -d out ] || mkdir out
	gcc -o $@ obj/avl.o $<

out/database_explorer: obj/database_explorer.o obj/comparators.o obj/avl.o
	[ -d out ] || mkdir out
	gcc -o out/database_explorer obj/comparators.o obj/database_explorer.o obj/avl.o

obj/%.o: lib/src/%.c lib/include/avl.h
	[ -d obj ] || mkdir obj
	gcc -I./lib/include/ -Werror -Wall -Wextra -c -o $@ $<

obj/database_explorer.o: showcase/database_explorer.c showcase/comparators.c showcase/explorer.h lib/include/avl.h
	[ -d obj ] || mkdir obj
	gcc -I./lib/include/ -c -o obj/database_explorer.o showcase/database_explorer.c
	gcc -I./lib/include/ -c -o obj/comparators.o showcase/comparators.c
