bin_PROGRAMS = can
can_SOURCES = canner.c html_cb.h main.h

EXTRA_DIST = http_cb.h.in main.h.in

BUILT_SOURCES = http_cb.h main.h

http_cb.h: http_cb.h.in
	sed 's#"#\\"#g' < $< | awk '{print "\"" $$0 "\","}' > $@

main.h: main.h.in
	sed 's#\\#\\\\#' < $< | sed 's#"#\\"#g' | awk '{print "\"" $$0 "\","}' > $@
