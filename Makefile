CFLAGS	:= -ggdb
LDFLAGS	:= -ggdb

all: main clean

main: example.o comm.o
	gcc $(LDFLAGS) -o example.exe example.o comm.o

example.o: example.c
	gcc $(CFLAGS) -c example.c

comm.o: comm.c comm.h
	gcc $(CFLAGS) -c comm.c -I.

.PHONY: clean
clean:
	rm *.o

