CC = gcc
CFLAGS = -Wall -std=c99 -g

un-utf8: un-utf8.o codepoints.o

un-utf8.o: codepoints.h
codepoints.o: codepoints.h

clean:
	rm -f un-utf8 un-utf8.o
	rm -f codepoints codepoints.o