.PHONY: all clean

.PRECIOUS: obj/%.o

all: out/test out/dbfilter

clean:
	rm -r out obj

out/test: obj/test.o obj/avl.o
	[ -d out ] || mkdir out
	cc -o $@ obj/avl.o $<

out/dbfilter: obj/dbfilter.o obj/comparators.o obj/avl.o
	[ -d out ] || mkdir out
	cc -o $@ obj/avl.o obj/comparators.o $<

obj/%.o: lib/src/%.c lib/include/avl.h
	[ -d obj ] || mkdir obj
	cc -I./lib/include/ -Werror -Wall -Wextra -c -o $@ $<

obj/dbfilter.o: showcase/dbfilter.c showcase/dbfilter.h lib/include/avl.h
	[ -d obj ] || mkdir obj
	cc -I./lib/include/ -c -o $@ $<

obj/comparators.o: showcase/comparators.c showcase/dbfilter.h
	[ -d obj ] || mkdir obj
	cc -I./lib/include/ -c -o $@ $<
