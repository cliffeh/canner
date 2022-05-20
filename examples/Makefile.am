CAN = ../can
SITE_DIR = static
STATIC_FILES = \
	static/index.html \
	static/style/style.css \
	static/foo/index.html \
	static/foo/bar.html \
	static/auden.txt

bin_PROGRAMS = site site-with-prefix site-with-custom
nodist_site_SOURCES = site.c site.h
nodist_site_with_prefix_SOURCES = site-with-prefix.c site-with-prefix.h
site_with_custom_SOURCES = api.c dice.c
nodist_site_with_custom_SOURCES = site-with-custom.c site-with-custom.h

BUILT_SOURCES = site.c site.h site-with-prefix.c site-with-prefix.h site-with-custom.c site-with-custom.h

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

site-with-custom.c site-with-custom.h: $(STATIC_FILES)
	$(CAN) -o site-with-custom.c -H site-with-custom.h $(SITE_DIR)
	echo -e '\n#define CANNER_HAS_CUSTOM_CALLBACKS\n' >> site-with-custom.h

CLEANFILES = $(BUILT_SOURCES)
