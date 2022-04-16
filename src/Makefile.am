bin_PROGRAMS = can
can_SOURCES = canner.c http_cb.h main.h

EXTRA_DIST = mkh.sh http_cb.h.in main.h.in

BUILT_SOURCES = http_cb.h main.h

http_cb.h: http_cb.h.in
	./mkh.sh $< $@

main.h: main.h.in
	./mkh.sh $< $@
