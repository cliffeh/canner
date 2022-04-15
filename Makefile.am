AUTOMAKE_OPTIONS = subdir-objects

EXAMPLES = \
	examples/Makefile \
	examples/static/style/style.css \
	examples/static/index.html \
	examples/static/foo/index.html \
	examples/static/foo/bar.html \
	examples/static/auden.txt

dist_doc_DATA = \
	LICENSE \
	README.md \
	TODO.md

EXTRA_DIST = $(EXAMPLES)

SUBDIRS = src test

dist_noinst_SCRIPTS = autogen.sh
