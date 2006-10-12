#! /bin/sh

libtoolize --force && aclocal && automake --add-missing --copy && autoconf
