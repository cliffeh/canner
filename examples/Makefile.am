CAN = ../can
SITE_DIR = static
STATIC_FILES = \
	static/index.html \
	static/style/style.css \
	static/foo/index.html \
	static/foo/bar.html \
	static/auden.txt

bin_PROGRAMS = site
site_SOURCES = site.h api.c
nodist_site_SOURCES = site.c

BUILT_SOURCES = site.c

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
site.c: $(STATIC_FILES) site.h
	$(CAN) $(SITE_DIR) site.h > $@

CLEANFILES = $(BUILT_SOURCES)
