CC = gcc
CFLAGS = -Wall -Wextra -p -g -Wfloat-equal -Wundef -Wconversion -Wunreachable-code  -pedantic
FASTFLAGS = -Ofast -march=native
BIN = bin
SRC = src
OBJ = $(BIN)/main.o $(BIN)/lambda.o $(BIN)/types.o $(BIN)/lexer.o $(BIN)/parser.o $(BIN)/infer.o $(BIN)/primitives.o
TEST_OBJECTS = $(BIN)/tests.o $(BIN)/lambda.o $(BIN)/types.o $(BIN)/lexer.o $(BIN)/parser.o $(BIN)/infer.o $(BIN)/primitives.o
LDFLAGS = -lreadline

all: $(BIN) lambda tests

$(BIN): 
	mkdir -p $(BIN)

lambda: $(OBJ)
	$(CC) $(CFLAGS) -o lambda $(OBJ) $(LDFLAGS)

tests: $(TEST_OBJECTS)
	$(CC) $(CFLAGS) -o tests $(TEST_OBJECTS) $(LDFLAGS)

$(BIN)/main.o: $(SRC)/main.c $(SRC)/lambda.h $(SRC)/parser.h $(SRC)/infer.h $(SRC)/primitives.h | $(BIN)
	$(CC) $(CFLAGS) -c $(SRC)/main.c -o $(BIN)/main.o

$(BIN)/tests.o: $(SRC)/tests.c $(SRC)/lambda.h $(SRC)/parser.h $(SRC)/infer.h $(SRC)/primitives.h | $(BIN)
	$(CC) $(CFLAGS) -c $(SRC)/tests.c -o $(BIN)/tests.o

$(BIN)/lambda.o: $(SRC)/lambda.c $(SRC)/lambda.h $(SRC)/types.h | $(BIN)
	$(CC) $(CFLAGS) -c $(SRC)/lambda.c -o $(BIN)/lambda.o

$(BIN)/types.o: $(SRC)/types.c $(SRC)/types.h | $(BIN)
	$(CC) $(CFLAGS) -c $(SRC)/types.c -o $(BIN)/types.o

$(BIN)/lexer.o: $(SRC)/lexer.c $(SRC)/lexer.h | $(BIN)
	$(CC) $(CFLAGS) -c $(SRC)/lexer.c -o $(BIN)/lexer.o

$(BIN)/parser.o: $(SRC)/parser.c $(SRC)/parser.h $(SRC)/lambda.h $(SRC)/lexer.h | $(BIN) 
	$(CC) $(CFLAGS) -c $(SRC)/parser.c -o $(BIN)/parser.o

$(BIN)/infer.o: $(SRC)/infer.c $(SRC)/infer.h $(SRC)/lambda.h $(SRC)/types.h | $(BIN)
	$(CC) $(CFLAGS) -c $(SRC)/infer.c -o $(BIN)/infer.o 

$(BIN)/primitives.o: $(SRC)/primitives.c $(SRC)/primitives.h $(SRC)/lambda.h | $(BIN)
	$(CC) $(CFLAGS) -c $(SRC)/primitives.c -o $(BIN)/primitives.o

clean:
	rm -f $(BIN)/*.o lambda tests 
	rm -r $(BIN) 2>/dev/null || true 

.PHONY: format
format:
	clang-format -i $(SRC)/*.c $(SRC)/*.h

.PHONY: all clean
