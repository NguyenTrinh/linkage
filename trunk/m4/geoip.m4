#Modified CHECK_ZLIB script found here http://autoconf-archive.cryp.to/check_zlib.html
#Copyright © 2004 Loic Dachary <loic@senga.org>
#Copyright © 2006 Christian Lundgren <lunke@lunke.se>

#This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

#See ../LICENSE for full version

AC_DEFUN([CHECK_GEOIP],
#
# Handle user hints
#
[AC_MSG_CHECKING(if geoip is wanted)
AC_ARG_WITH(geoip,
[  --with-geoip=DIR root directory path of geoip installation [defaults to
                    /usr/local or /usr if not found in /usr/local]
  --without-geoip to disable geoip usage completely],
[if test "$withval" != no ; then
  AC_MSG_RESULT(yes)
  if test -d "$withval"
  then
    GEOIP_HOME="$withval"
  else
    AC_MSG_WARN([Sorry, $withval does not exist, checking usual places])
  fi
else
  AC_MSG_RESULT(no)
fi])

GEOIP_HOME=/usr/local
if test ! -f "${GEOIP_HOME}/include/GeoIP.h"
then
        GEOIP_HOME=/usr
fi

#
# Locate geoip, if wanted
#
if test -n "${GEOIP_HOME}"
then
        GEOIP_OLD_LDFLAGS=$LDFLAGS
        GEOIP_OLD_CPPFLAGS=$CPPFLAGS
        LDFLAGS="$LDFLAGS -L${GEOIP_HOME}/lib -lGeoIP"
        CPPFLAGS="$CPPFLAGS -I${GEOIP_HOME}/include"
        AC_LANG_SAVE
        AC_LANG_C
        AC_CHECK_LIB(GeoIP, GeoIP_country_code_by_addr, [geoip_cv_libgeoip=yes], [geoip_cv_libgeoip=no])
        AC_CHECK_HEADER(GeoIP.h, [geoip_cv_geoip_h=yes], [geoip_cv_geoip_h=no])
        AC_LANG_RESTORE
        if test "$geoip_cv_libgeoip" = "yes" -a "$geoip_cv_geoip_h" = "yes"
        then
                #
                # If both library and header were found, use them
                #
                USE_GEOIP=yes
                AC_CHECK_LIB(GeoIP, GeoIP_country_code_by_addr)
                AC_MSG_CHECKING(geoip in ${GEOIP_HOME})
                AC_MSG_RESULT(ok)
        else
                #
                # If either header or library was not found, revert and bomb
                #
                AC_MSG_CHECKING(geoip in ${GEOIP_HOME})
                LDFLAGS="$GEOIP_OLD_LDFLAGS"
                CPPFLAGS="$GEOIP_OLD_CPPFLAGS"
                AC_MSG_RESULT(failed)
        fi
fi

])
