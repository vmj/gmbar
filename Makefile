
CC=gcc
CFLAGS=-Wall -Os
LDFLAGS=

TARGETS=gmmembar

all: gmmembar

libgmbar.o: libgmbar.h libgmbar.c
	$(CC) $(CFLAGS) -c libgmbar.c

gmmembar: libgmbar.o gmmembar.c
	$(CC) $(CFLAGS) -c gmmembar.c
	$(CC) $(LDFLAGS) -o gmmembar libgmbar.o gmmembar.o

clean:
	-@rm *~ *.o

distclean: clean
	-@rm $(TARGETS)
