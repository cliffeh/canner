bin_PROGRAMS = can
can_SOURCES = canner.c options.c
nodist_can_SOURCES = http_cb.h main.h

EXTRA_DIST = mkh.sh http_cb.h.in main.h.in

BUILT_SOURCES = http_cb.h main.h

http_cb.h: http_cb.h.in
	$(srcdir)/mkh.sh $< $@

main.h: main.h.in
	$(srcdir)/mkh.sh $< $@

dist_doc_DATA = \
	LICENSE \
	README.md \
	TODO.md

SUBDIRS = examples test

dist_noinst_SCRIPTS = autogen.sh

CLEANFILES = $(BUILT_SOURCES)
