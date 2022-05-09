CAN = ../can
SITE_DIR = static
STATIC_FILES = \
	static/index.html \
	static/style/style.css \
	static/foo/index.html \
	static/foo/bar.html \
	static/auden.txt

bin_PROGRAMS = site site-with-prefix
# site_SOURCES = api.c
nodist_site_SOURCES = site.c site.h
nodist_site_with_prefix_SOURCES = site-with-prefix.c site-with-prefix.h

BUILT_SOURCES = site.c site.h site-with-prefix.c site-with-prefix.h

EXTRA_DIST = $(STATIC_FILES)

AUTOMAKE_OPTIONS = subdir-objects

check_PROGRAMS = test/test-dice
test_test_dice_SOURCES = test/test-dice.c dice.c

TESTS = $(check_PROGRAMS)

# note: to generate a binary that statically links libevent you
#   can do something like the following if your compiler/linker
#   support -Bstatic/-Bdynamic:
#
#   $(CC) -o $@ $< -Wl,-Bstatic -levent -Wl,-Bdynamic
#
site.c site.h: $(STATIC_FILES)
	$(CAN) -o site.c -H site.h $(SITE_DIR)

site-with-prefix.c site-with-prefix.h: $(STATIC_FILES)
	$(CAN) -o site-with-prefix.c -H site-with-prefix.h -p static $(SITE_DIR)

CLEANFILES = $(BUILT_SOURCES)
