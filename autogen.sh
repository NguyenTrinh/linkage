#! /bin/sh

intltoolize --force --copy && \
libtoolize --force && \
aclocal -I m4 && \
autoheader && \
automake --add-missing --copy && \
autoconf && \
./configure "$@"
