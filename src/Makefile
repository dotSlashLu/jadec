CC=gcc
CCFLAGS=-g -Wall -I.. -O3
TARGETS=parser.c lexer.c jadec.c pool.c

jadec.out:
	$(CC) $(CCFLAGS) $(TARGETS) -o jadec.out

test: clean jadec.out
	./jadec.out test.in

memtest: clean jadec.out
	valgrind --leak-check=full -v ./jadec.out test.in

clean:
	rm -f *.out *.o *.a
