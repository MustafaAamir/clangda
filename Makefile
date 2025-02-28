CC = gcc
CFLAGS = -Wall -Wextra -p -g -Wfloat-equal -Wundef -Wconversion -Wunreachable-code  -pedantic
FASTFLAGS = -Ofast -march=native
OBJECTS = main.o lambda.o types.o lexer.o parser.o infer.o primitives.o
TEST_OBJECTS = tests.o lambda.o types.o lexer.o parser.o infer.o primitives.o

all: lambda tests

lambda: $(OBJECTS)
	$(CC) $(CFLAGS) -o lambda $(OBJECTS) $(LDFLAGS)

tests: $(TEST_OBJECTS)
	$(CC) $(CFLAGS) -o tests $(TEST_OBJECTS) $(LDFLAGS)

main.o: main.c lambda.h parser.h infer.h primitives.h
	$(CC) $(CFLAGS) -c main.c

tests.o: tests.c lambda.h parser.h infer.h primitives.h
	$(CC) $(CFLAGS) -c tests.c

lambda.o: lambda.c lambda.h types.h
	$(CC) $(CFLAGS) -c lambda.c

types.o: types.c types.h
	$(CC) $(CFLAGS) -c types.c

lexer.o: lexer.c lexer.h
	$(CC) $(CFLAGS) -c lexer.c

parser.o: parser.c parser.h lambda.h lexer.h
	$(CC) $(CFLAGS) -c parser.c

infer.o: infer.c infer.h lambda.h types.h
	$(CC) $(CFLAGS) -c infer.c

primitives.o: primitives.c primitives.h lambda.h
	$(CC) $(CFLAGS) -c primitives.c

clean:
	rm -f *.o lambda tests 

.PHONY: all clean
