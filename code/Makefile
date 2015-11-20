CFLAGS = -Wall -O2 -ansi -pedantic -Wno-unused-result -lreadline
CC = gcc
RM = rm
#---------------------------------------------------

all: ep3 clean

ep3: main.o io.o utils.o
	$(CC) -o ep3 main.o utils.o io.o $(CFLAGS)

main.o: main.c io.h
	$(CC) -c main.c $(CFLAGS)

io.o: io.h io.c utils.h
	$(CC) -c io.c $(CFLAGS)

utils.o: utils.h utils.c
	$(CC) -c utils.c $(CFLAGS)


clean:
	mkdir -p ../bin
	mv ep3 ../bin/
	rm *.o
