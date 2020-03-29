CC = gcc
CFLAGS = -I. -g
OBJECTS = memFunctions.o
.SUFFIXES: .c .o

all: oss userP

oss: oss.o $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ oss.o $(OBJECTS)

userP: userP.o
	$(CC) $(CFLAGS) -o $@ userP.o

.c.o:
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm *.o oss userP
	
