CAN = ../src/can
STATIC_DIR = static
STATIC_FILES = \
	static/index.html \
	static/style/style.css \
	static/foo/index.html \
	static/foo/bar.html \
	static/auden.txt

bin_PROGRAMS = site
site_SOURCES = site.c

EXTRA_DIST = $(STATIC_FILES)

# note: to generate a binary that statically links libevent you
#   can do something like the following if your compiler/linker
#   support -Bstatic/-Bdynamic:
#
#   $(CC) -o $@ $< -Wl,-Bstatic -levent -Wl,-Bdynamic
#
site.c: $(STATIC_FILES)
	$(CAN) $(STATIC_DIR) > $@