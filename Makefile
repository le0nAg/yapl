CC = gcc
CFLAGS = -Wall -g -Isrc/include
LEX = flex
YACC = bison

# Target executable
TARGET = interpreter

# Source files (now in src/)
SOURCES = src/parser.tab.c src/lex.yy.c src/ast.c src/interpreter.c
OBJECTS = $(SOURCES:.c=.o)

# Header files (now in src/include/)
HEADERS = src/include/ast.h src/parser.tab.h src/include/interpreter.h

all: $(TARGET)

# Generate parser (output goes into src/)
src/parser.tab.c src/parser.tab.h: src/parser.y src/include/ast.h
	$(YACC) -d -o src/parser.tab.c src/parser.y

# Generate scanner (output goes into src/)
src/lex.yy.c: src/scanner.l src/parser.tab.h
	$(LEX) -o src/lex.yy.c src/scanner.l

# Compile AST implementation
src/ast.o: src/ast.c src/include/ast.h
	$(CC) $(CFLAGS) -c src/ast.c -o src/ast.o

# Compile interpreter
src/interpreter.o: src/interpreter.c src/include/interpreter.h src/include/ast.h
	$(CC) $(CFLAGS) -c src/interpreter.c -o src/interpreter.o

# Compile parser
src/parser.tab.o: src/parser.tab.c src/include/ast.h
	$(CC) $(CFLAGS) -c src/parser.tab.c -o src/parser.tab.o

# Compile scanner
src/lex.yy.o: src/lex.yy.c src/parser.tab.h src/include/ast.h
	$(CC) $(CFLAGS) -c src/lex.yy.c -o src/lex.yy.o

# Link everything
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) -lfl

# Test with example program
test: $(TARGET)
	./$(TARGET) prog_files/input.prog

# Clean generated files
clean:
	rm -f $(TARGET) $(OBJECTS)
	rm -f src/parser.tab.c src/parser.tab.h src/lex.yy.c

.PHONY: all test clean
