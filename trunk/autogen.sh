#! /bin/sh

rev=`LC_ALL=C svn info $0 | awk '/^Revision: / {printf "%i\n", $2}'`
sed -e "s/@REVISION@/${rev}/g" < "configure.ac.in" > "configure.ac"

intltoolize --force --copy && \
libtoolize --force && \
aclocal && \
autoheader && \
automake --add-missing --copy && \
autoconf && \
./configure "$@"
