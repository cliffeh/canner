#!/bin/sh

if test ! $# = 2; then
   echo 'usage: mkh.sh source target' >&2
   exit 1
fi

sed 's,\\,\\\\,g' < "$1" | sed 's,",\\",g' | awk '{print "\"" $0 "\","}' > "$2"
