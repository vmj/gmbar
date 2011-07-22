
all: subdirs

clean: subdirs
	-@rm *~ 2>/dev/null || true

distclean: subdirs clean

dist: distclean index.html
	@$(MAKE) -C doc -s all
	@$(MAKE) -s dist-internal VERSION=`grep _version src/version.h |sed -e 's/.* "//' -e 's/";//'`

index.html:
	@rst2html.py README.rst index.html

dist-internal:
	-@rm -rf gmbar-$(VERSION) gmbar-$(VERSION).tar.gz 2>/dev/null || true
	@mkdir gmbar-$(VERSION)
	@cp -r LICENSE.txt Makefile README.rst doc img src gmbar-$(VERSION)/
	@tar czf gmbar-$(VERSION).tar.gz gmbar-$(VERSION)
	@rm -rf gmbar-$(VERSION)

SUBDIRS = src doc

.PHONY: subdirs $(SUBDIRS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	@$(MAKE) -C $@ $(MAKEFLAGS) $(MAKECMDGOALS)
