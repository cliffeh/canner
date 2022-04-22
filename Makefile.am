bin_PROGRAMS = can
can_SOURCES = canner.c canner.h util.c
nodist_can_SOURCES = static_cb.h main.h

EXTRA_DIST = mkh.sh static_cb.h.in main.h.in

BUILT_SOURCES = static_cb.h main.h

static_cb.h: static_cb.h.in
	$(srcdir)/mkh.sh $< $@

main.h: main.h.in
	$(srcdir)/mkh.sh $< $@

dist_doc_DATA = \
	LICENSE \
	README.md \
	TODO.md

SUBDIRS = . examples test

dist_noinst_SCRIPTS = autogen.sh

CLEANFILES = $(BUILT_SOURCES)
