CC=clang
NAME=chalk
WARN=-Wall -Wextra -Werror
STD=-pedantic -std=c89
CFLAGS=$(WARN) $(STD)
LIBS=-lm

all: main obj
	mkdir -p bin
	$(CC) o/*.o -o bin/$(NAME) $(LIBS)

main:
	mkdir -p o
	$(CC) src/main.c -o o/main.o -c $(CFLAGS)

obj:
	mkdir -p o
	$(CC) src/obj.c -o o/obj.o -c $(CFLAGS)

run: all
	./bin/$(NAME)

clean:
	rm -f bin/*
	rm -f o/*
