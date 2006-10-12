#Modified CHECK_ZLIB script found here http://autoconf-archive.cryp.to/check_zlib.html
#Copyright © 2004 Loic Dachary <loic@senga.org>
#Copyright © 2006 Christian Lundgren <lunke@lunke.se>

#This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

#See ../LICENSE for full version

AC_DEFUN([CHECK_LIBNOTIFY],
#
# Handle user hints
#
[AC_MSG_CHECKING(if libnotify is wanted)
AC_ARG_WITH(libnotify,
[  --with-libnotify=DIR root directory path of libnotify installation [defaults to
                    /usr/local or /usr if not found in /usr/local]
  --without-libnotify to disable libnotify usage completely],
[if test "$withval" != no ; then
  AC_MSG_RESULT(yes)
  if test -d "$withval"
  then
    LIBNOTIFY_HOME="$withval"
  else
    AC_MSG_WARN([Sorry, $withval does not exist, checking usual places])
  fi
else
  AC_MSG_RESULT(no)
fi])

LIBNOTIFY_HOME=/usr/local
if test ! -f "${LIBNOTIFY_HOME}/include/libnotify/notify.h"
then
        LIBNOTIFY_HOME=/usr
fi

#
# Locate libnotify, if wanted
#
if test -n "${LIBNOTIFY_HOME}"
then
        LIBNOTIFY_OLD_LDFLAGS=$LDFLAGS
        LIBNOTIFY_OLD_CPPFLAGS=$CPPFLAGS
        LDFLAGS="$LDFLAGS -L${LIBNOTIFY_HOME}/lib -lnotify"
        CPPFLAGS="$CPPFLAGS -I${LIBNOTIFY_HOME}/include"
        AC_LANG_SAVE
        AC_LANG_C
        AC_CHECK_LIB(notify, notify_notification_newr, [libnotify_cv_liblibnotify=yes], [libnotify_cv_liblibnotify=no])
        AC_CHECK_HEADER(notify.h, [libnotify_cv_libnotify_h=yes], [libnotify_cv_libnotify_h=no])
        AC_LANG_RESTORE
        if test "$libnotify_cv_liblibnotify" = "yes" -a "$libnotify_cv_libnotify_h" = "yes"
        then
                #
                # If both library and header were found, use them
                #
                USE_LIBNOTIFY=yes
                AC_CHECK_LIB(notify, notify_notification_new)
                AC_MSG_CHECKING(libnotify in ${LIBNOTIFY_HOME})
                AC_MSG_RESULT(ok)
        else
                #
                # If either header or library was not found, revert and bomb
                #
                AC_MSG_CHECKING(libnotify in ${LIBNOTIFY_HOME})
                LDFLAGS="$LIBNOTIFY_OLD_LDFLAGS"
                CPPFLAGS="$LIBNOTIFY_OLD_CPPFLAGS"
                AC_MSG_RESULT(failed)
        fi
fi

])
