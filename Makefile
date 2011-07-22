export
DESTDIR ?=
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
MANDIR ?= $(PREFIX)/man
MAN1DIR ?= $(MANDIR)/man1

all: subdirs

clean: subdirs
	-@rm *~ 2>/dev/null || true

distclean: subdirs clean
	-@rm gmbar-*.tar.gz 2>/dev/null || true

dist: subdirs
	@$(MAKE) -s dist-internal VERSION=`grep _version src/version.h |sed -e 's/.* "//' -e 's/";//'`

install: subdirs

dist-internal:
	-@rm -rf gmbar-$(VERSION) gmbar-$(VERSION).tar.gz 2>/dev/null || true
	@mkdir gmbar-$(VERSION)
	@cp -r LICENSE.txt Makefile README.rst doc src gmbar-$(VERSION)/
	@tar czf gmbar-$(VERSION).tar.gz gmbar-$(VERSION)
	@rm -rf gmbar-$(VERSION)

SUBDIRS = src doc

.PHONY: subdirs $(SUBDIRS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	@$(MAKE) -C $@ $(MAKEFLAGS) $(MAKECMDGOALS)
