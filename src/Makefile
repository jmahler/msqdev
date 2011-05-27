
INCLUDE=

OBJECTS = 
CC=gcc
CFLAGS=-Wall -ansi -pedantic $(INCLUDE)

all: msqdev

doc: *.c *.h 
	doxygen  # using Doxyfile 

msqdev: main.c $(OBJECTS) msqserial.h
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

clean:
	-rm -f main msqdev
	-rm -f $(OBJECTS)
	-rm -fr doc

