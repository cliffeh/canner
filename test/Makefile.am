check_SCRIPTS = it-generates-code.test it-compiles.test it-runs.test it-listens.test

TEST_EXTENSIONS = .test

TESTS = $(check_SCRIPTS)

dist_noinst_SCRIPTS = $(check_SCRIPTS)

STATIC_FILES = \
	../examples/static/style/style.css \
	../examples/static/index.html \
	../examples/static/foo/index.html \
	../examples/static/foo/bar.html \
	../examples/static/auden.txt

it-generates-code.log: $(STATIC_FILES)

it-compiles.log: it-generates-code.log

it-runs.log: it-compiles.log

it-listens.log: it-runs.log

example-site: example-site.c
	$(CC) -o $@ $< -levent

distclean-local:
	rm -f example-site.c example-site
