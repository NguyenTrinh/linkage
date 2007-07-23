m4_define(linkage_major_version, 0)
m4_define(linkage_minor_version, 1)
m4_define(linkage_micro_version, 2)

m4_define(linkage_version, linkage_major_version().linkage_minor_version().linkage_micro_version())

AC_INIT([linkage], [linkage_version()])
AC_CONFIG_SRCDIR(src/main.cc)
AM_INIT_AUTOMAKE([$PACKAGE_NAME], [$PACKAGE_VERSION])
AC_CONFIG_HEADERS([config.h])

AC_DEFINE([LINKAGE_VERSION_MAJOR], [linkage_major_version()], [Major version])
AC_DEFINE([LINKAGE_VERSION_MINOR], [linkage_minor_version()], [Minor version])
AC_DEFINE([LINKAGE_VERSION_MICRO], [linkage_micro_version()], [Micro version])

PKG_CHECK_MODULES(GTKMM, gtkmm-2.4 >= 2.10)
AC_SUBST(GTKMM_CFLAGS)
AC_SUBST(GTKMM_LIBS)

PKG_CHECK_MODULES(GTHREAD, gthread-2.0 >= 2.10)
AC_SUBST(GTHREAD_CFLAGS)
AC_SUBST(GTHREAD_LIBS)

PKG_CHECK_MODULES(LIBTORRENT, libtorrent >= 0.12)
AC_SUBST(LIBTORRENT_CFLAGS)
AC_SUBST(LIBTORRENT_LIBS)

PKG_CHECK_MODULES(LIBNOTIFY, libnotify >= 0.4.2)
AC_SUBST(LIBNOTIFY_CFLAGS)
AC_SUBST(LIBNOTIFY_LIBS)

PKG_CHECK_MODULES(LIBCURL, libcurl >= 7.14.0)
AC_SUBST(LIBCURL_CFLAGS)
AC_SUBST(LIBCURL_LIBS)

PKG_CHECK_MODULES(DBUSGLIB, dbus-glib-1 >= 0.70)
AC_SUBST(DBUSGLIB_CFLAGS)
AC_SUBST(DBUSGLIB_LIBS)

AC_ARG_ENABLE([upnp],
    [AS_HELP_STRING([--enable-upnp], [Enable libupnp (default=disabled)])],
    [enable_upnp="$enableval"],
    [enable_upnp="no"]
)

if test "$enable_upnp" = "yes"; then
    PKG_CHECK_MODULES([LIBUPNP], [libupnp >= 1.4.1])
    AC_DEFINE([BUILDUPNP], [], [Define when building with libupnp])    
    BUILDUPNP="true"
else
    BUILDUPNP="false"
fi

AC_SUBST([ENABLE_LIBUPNP])
AM_CONDITIONAL([BUILDUPNP], [test "$enable_upnp" = "yes"])

AC_SUBST(LIBUPNP_CFLAGS)
AC_SUBST(LIBUPNP_LIBS)

AC_PROG_CC()
AC_PROG_CXX()
AC_DISABLE_STATIC()
AC_PROG_LIBTOOL()
AC_PROG_INSTALL()

AC_OUTPUT(Makefile src/Makefile plugins/Makefile lib/Makefile lib/linkage/Makefile plugins/notification/Makefile plugins/trayicon/Makefile plugins/upnp/Makefile  data/Makefile data/flags/Makefile)

