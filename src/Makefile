DESTDIR ?=
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

CC = gcc
CFLAGS = -Wall -Os -D_GNU_SOURCE
LDFLAGS =

TOOLS_SRC = $(wildcard gm*bar.c)
TOOLS = $(basename $(TOOLS_SRC))

LIBS_SRC = $(filter-out $(TOOLS_SRC),$(wildcard *.c))
LIBS_OBJ = $(LIBS_SRC:.c=.o)

all: $(TOOLS)

clean:
	-@rm *~ *.o 2>/dev/null || true

distclean: clean
	-@rm -rf $(TOOLS) 2>/dev/null || true

dist:

install: $(TOOLS)
	mkdir -p $(DESTDIR)$(BINDIR)
	cp $(TOOLS) $(DESTDIR)$(BINDIR)/

%.o: %.c %.h
	$(CC) $(CFLAGS) -o $@ -c $<

gm%bar: gm%bar.c version.h $(LIBS_OBJ)
	$(CC) $(CFLAGS) -o $@.o -c $<
	$(CC) $(LDFLAGS) -o $@ $@.o $(LIBS_OBJ)
