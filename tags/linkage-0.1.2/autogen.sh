#! /bin/sh

libtoolize --force && aclocal && autoheader && automake --add-missing --copy && autoconf && ./configure "$@"