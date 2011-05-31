
INCLUDE=

OBJECTS= 
CC=g++
CFLAGS=-Wall -ansi -pedantic $(INCLUDE)

all: msqdev

doc: *.cpp *.h msqdev.cpp
	doxygen  # using Doxyfile 

msqdev: msqdev.cpp MSQSerial.h MSQData.h MSQData/Table.h MSQRealTime.h MSQUtils.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -f msqdev
	-rm -f $(OBJECTS)
	-rm -fr doc

