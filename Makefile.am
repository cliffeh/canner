bin_PROGRAMS = can
can_SOURCES = callbacks.c main.c util.c
nodist_can_SOURCES = main.h site.h static_cb.h

EXTRA_DIST = mkh.sh main.h.in site.h.in static_cb.h.in

BUILT_SOURCES = site.h static_cb.h main.h

main.h: main.h.in
	$(srcdir)/mkh.sh $< $@

site.h: site.h.in
	$(srcdir)/mkh.sh $< $@

static_cb.h: static_cb.h.in
	$(srcdir)/mkh.sh $< $@

dist_doc_DATA = \
	LICENSE \
	README.md \
	TODO.md

SUBDIRS = . examples test

dist_noinst_SCRIPTS = autogen.sh

CLEANFILES = $(BUILT_SOURCES)
