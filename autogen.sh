#!/bin/sh
set -x
rm -rf autom4te.cache
libtoolize --automake -f
aclocal
autoheader
automake --foreign --add-missing --copy
autoconf
