#!/bin/tcsh

aclocal
autoheader
glibtoolize
autoconf
automake --add-missing
./configure
make dist
