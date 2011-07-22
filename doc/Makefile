DESTDIR ?=
PREFIX ?= /usr/local
MANDIR ?= $(PREFIX)/man
MAN1DIR ?= $(MANDIR)/man1

MAN_TXT = $(wildcard gm*bar.[1-8].txt)
MAN_XML = $(MAN_TXT:.txt=.xml)
MAN = $(MAN_XML:.xml=)

MAN1 = $(filter %.1,$(MAN))

all: man

clean:
	-@rm *~ $(MAN_XML) 2>/dev/null || true

distclean: clean
	-@rm $(MAN) 2>/dev/null || true

dist:

install: man
	mkdir -p $(DESTDIR)$(MAN1DIR)
	cp $(MAN1) $(DESTDIR)$(MAN1DIR)/

man: $(MAN)

%: %.xml
	xmlto man $<

%.xml: %.txt
	asciidoc -b docbook -d manpage -o $@ $<
