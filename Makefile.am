bin_PROGRAMS = can
can_SOURCES = callbacks.c main.c util.c
can_LDADD = popt/src/libpopt.la
nodist_can_SOURCES = main_template.h site_template.h static_cb_template.h

EXTRA_DIST = mkh.sh main_template.in site_template.in static_cb_template.in

BUILT_SOURCES = main_template.h site_template.h static_cb_template.h

main_template.h: main_template.in
	$(srcdir)/mkh.sh $< $@

site_template.h: site_template.in
	$(srcdir)/mkh.sh $< $@

static_cb_template.h: static_cb_template.in
	$(srcdir)/mkh.sh $< $@

dist_doc_DATA = \
	LICENSE \
	README.md \
	TODO.md

SUBDIRS = popt . examples test

dist_noinst_SCRIPTS = autogen.sh

CLEANFILES = $(BUILT_SOURCES)
