AUTOMAKE_OPTIONS = subdir-objects

EXAMPLES = \
	examples/README.md \
	examples/static/style/style.css \
	examples/static/index.html \
	examples/static/foo/index.html \
	examples/static/foo/bar.html \
	examples/static/auden.txt

dist_doc_DATA = \
	LICENSE \
	README.md \
	TODO.md \
	$(EXAMPLES)

SUBDIRS = src test

dist_noinst_SCRIPTS = autogen.sh
