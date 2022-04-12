bin_PROGRAMS = can
can_SOURCES = canner.c html_cb.h

EXTRA_DIST = html_cb.h.in

BUILT_SOURCES = html_cb.h

html_cb.h: html_cb.h.in
	sed 's#"#\\"#g' < $< | awk '{print "\"" $$0 "\","}' > $@
