CC = gcc
CFLAGS = -Wall -Os -D_GNU_SOURCE
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

src/common.o: src/common.h src/common.c
	$(CC) $(CFLAGS) -o src/common.o -c src/common.c

src/buffer.o: src/buffer.h src/buffer.c
	$(CC) $(CFLAGS) -o src/buffer.o -c src/buffer.c

src/readfile.o: src/readfile.h src/readfile.c
	$(CC) $(CFLAGS) -o src/readfile.o -c src/readfile.c

src/log.o: src/log.h src/log.c
	$(CC) $(CFLAGS) -o src/log.o -c src/log.c

bin/%: src/%.c src/libgmbar.o src/common.o src/readfile.o src/log.o src/buffer.o src/version.h
	-@mkdir bin 2>/dev/null || true
	$(CC) $(CFLAGS) -o src/$*.o -c $<
	$(CC) $(LDFLAGS) -o $@ src/$*.o src/common.o src/readfile.o src/log.o src/libgmbar.o src/buffer.o

clean:
	-@rm *~ src/*~ src/*.o 2>/dev/null || true

distclean: clean
	-@rm -rf $(TARGETS) 2>/dev/null || true

doc:
	@rst2html.py README.rst index.html

dist: distclean doc
	@$(MAKE) -s dist-internal VERSION=`grep _version src/version.h |sed -e 's/.* "//' -e 's/";//'`

dist-internal:
	-@rm -rf gmbar-$(VERSION) gmbar-$(VERSION).tar.gz 2>/dev/null || true
	@mkdir gmbar-$(VERSION)
	@cp -r LICENSE.txt Makefile README.rst img src gmbar-$(VERSION)/
	@tar czf gmbar-$(VERSION).tar.gz gmbar-$(VERSION)
	@rm -rf gmbar-$(VERSION)
