#! /bin/sh

rev=`LC_ALL=C svn info $0 | awk '/^Revision: / {printf "%i\n", $2}'`
sed -e "s/@REVISION@/${rev}/g" < "configure.ac.in" > "configure.ac"

libtoolize --force && aclocal && automake --add-missing --copy && autoconf
