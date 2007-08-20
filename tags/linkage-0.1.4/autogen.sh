#! /bin/sh

intltoolize --force --copy && \
libtoolize --force && \
aclocal && \
autoheader && \
automake --add-missing --copy && \
autoconf && \
./configure "$@"
