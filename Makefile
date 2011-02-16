CC = gcc
CFLAGS = -Wall -Os
LDFLAGS =

SOURCES = $(wildcard src/gm*bar.c)
TARGETS = $(addprefix bin/,$(notdir $(basename $(SOURCES))))

all: $(TARGETS)

help:
	@echo "make all       - compile all tools"
	@echo "make clean     - remove temporary files"
	@echo "make distclean - remove compiled tools and temporary files"

src/libgmbar.o: src/libgmbar.h src/libgmbar.c
	$(CC) $(CFLAGS) -o src/libgmbar.o -c src/libgmbar.c

bin/%: src/%.c src/libgmbar.o
	-@mkdir bin 2>/dev/null || true
	$(CC) $(CFLAGS) -o src/$*.o -c $<
	$(CC) $(LDFLAGS) -o $@ src/$*.o src/libgmbar.o

clean:
	-@rm *~ src/*~ src/*.o 2>/dev/null

distclean: clean
	-@rm -rf $(TARGETS) 2>/dev/null
