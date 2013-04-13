#!/bin/sh

# Responsible for running all the necessary programs in
# the GNU Autotools package to properly setup the build system
# and generate the necessarily build files for you to type
#   configure
#   make
#   make install

# Stop on error
set -e

rm -rf autom4te.cache/

echo "-- Running aclocal"
aclocal -I build/m4

echo "-- Running libtoolize"
libtoolize -f -c

echo "-- Running aclocal"
aclocal -I build/m4

echo "-- Running autoconf"
autoconf -f

echo "-- Running autoheader"
autoheader

echo "-- Running automake"
automake -a -c -f -Wno-portability --foreign
