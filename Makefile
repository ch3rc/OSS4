CC = gcc
CFLAGS = -I. -g -Wall
.SUFFIXES: .c .o

all: oss userP

oss: oss.o
	$(CC) $(CFLAGS) -o $@ oss.o -lpthread

userP: userP.o
	$(CC) $(CFLAGS) -o $@ userP.o -lpthread

.c.o:
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm *.o oss userP
	
