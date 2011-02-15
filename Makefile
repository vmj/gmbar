
CC=gcc
CFLAGS=-Wall -Os
LDFLAGS=

TARGETS=gmmembar gmcpubar

all: gmmembar gmcpubar

libgmbar.o: libgmbar.h libgmbar.c
	$(CC) $(CFLAGS) -c libgmbar.c

gmmembar: libgmbar.o gmmembar.c
	$(CC) $(CFLAGS) -c gmmembar.c
	$(CC) $(LDFLAGS) -o gmmembar libgmbar.o gmmembar.o

gmcpubar: libgmbar.o gmcpubar.c
	$(CC) $(CFLAGS) -c gmcpubar.c
	$(CC) $(LDFLAGS) -o gmcpubar libgmbar.o gmcpubar.o

clean:
	-@rm *~ *.o

distclean: clean
	-@rm $(TARGETS)
