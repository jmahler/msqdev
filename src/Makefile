
CC=g++
CFLAGS=-Wall -g -ansi -pedantic $(INCLUDE)
INCLUDE=-I../include
OBJECTS= 

all: msqdev

msqdev: msqdev.cpp
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -f msqdev
	-rm -f $(OBJECTS)
	-rm -fr doc

