CC=gcc
CCFLAGS=-g -Wall
TARGETS=parser.c lexer.c

jadec.out:
	$(CC) $(CCFLAGS) $(TARGETS) -o jadec.out

test: clean jadec.out
	./jadec.out test.in

clean:
	rm -f *.out *.o *.a
