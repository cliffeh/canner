# canner
Static web sites in a single binary.

## What is it?
canner takes a collection of files and generates C source code for a single binary that serves them up as a website.

## How do I use it?

```bash
# generate source for static site files in the `examples` directory
can -o site.c examples
# compile it
cc -o site site.c -levent
# run it, listening on 127.0.0.1:8080
./site -H 127.0.0.1 -p 8080
```

The full list of options for `can`:
```
Usage: can [OPTION...] DIR
Options:
  -H, --header[=FILE]     also generate a C header file (FILE defaults to "site.h")
  -o, --outfile=FILE      output file name (default: "-")
  -p, --prefix=STRING     URI prefix (default: "")

Help options:
  -?, --help              Show this help message
      --usage             Display brief usage message
```

The full list of options for the binaries it generates:
```
usage: site [ OPTS ]
 -H      address to bind (default: 0.0.0.0)
 -p      port to bind (default: 8080)
 -v      enable verbose debugging
 -h      print a brief help message and exit
```

## How do I build it?

### Dependencies
canner depends on **libpopt** for argument parsing. The source code it generates depends on **libevent** for serving http traffic.

### Building
canner uses the autotools build system, so building it should be as simple as:

```bash
# you should only need to do this if you've cloned directly from git;
# distribution tarballs should come with everything they need already
./autogen.sh

./configure
make
make install
```

This will generate the binary `can` and install it.

It also has a set of basic unit tests that you can run using `make check`.

## Why did you do this?
Because I could...but also, this project was heavily inspired by https://j3s.sh/thought/my-website-is-one-binary.html.
