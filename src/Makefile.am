bin_PROGRAMS = can
can_SOURCES = canner.c html_cb.h main.h

EXTRA_DIST = html_cb.h.in main.h.in

BUILT_SOURCES = html_cb.h main.h

html_cb.h: html_cb.h.in
	sed 's#"#\\"#g' < $< | awk '{print "\"" $$0 "\","}' > $@

main.h: main.h.in
	sed 's#\\#\\\\#' < $< | sed 's#"#\\"#g' | awk '{print "\"" $$0 "\","}' > $@
