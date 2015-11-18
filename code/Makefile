CFLAGS = -Wall -O2 -ansi -pedantic -Wno-unused-result -lreadline
CC = gcc
RM = rm
#---------------------------------------------------

all: ep3 clean

ep3: main.o utils.o
	$(CC) -o ep2 main.o utils.o $(CFLAGS)

main.o: main.c utils.h
	$(CC) -c main.c $(CFLAGS)

utils.o: utils.h utils.c
	$(CC) -c utils.c $(CFLAGS)

clean:
	mkdir -p ../bin
	mv ep2 ../bin/
	rm *.o
