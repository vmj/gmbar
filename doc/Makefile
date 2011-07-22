
MAN_TXT = $(wildcard gm*bar.[1-8].txt)
MAN_XML = $(MAN_TXT:.txt=.xml)
MAN = $(MAN_XML:.xml=)

all: man

clean:
	-@rm *~ $(MAN_XML) 2>/dev/null || true

distclean: clean
	-@rm $(MAN) 2>/dev/null || true

dist:

man: $(MAN)

%: %.xml
	xmlto man $<

%.xml: %.txt
	asciidoc -b docbook -d manpage -o $@ $<
