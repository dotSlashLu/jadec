CC=gcc
CCFLAGS=-g -Wall
TARGETS=parser.c lexer.c jadec.c pool.c btree.c

config.h:
	cd .. && $(MAKE) configure

jadec.out: config.h
	$(CC) $(CCFLAGS) $(TARGETS) -o jadec.out

test: jadec.out
	./jadec.out test.in
	$(MAKE) clean

memtest: jadec.out
	valgrind --leak-check=full -v ./jadec.out test.in
	$(MAKE) clean

clean:
	rm -f *.out *.o *.a config.h

.PHONY: test memtest clean
