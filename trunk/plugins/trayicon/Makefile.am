trayicondir = $(libdir)/linkage/plugins

trayicon_LTLIBRARIES = trayicon.la

noinst_HEADERS = PluginTrayIcon.hh

trayicon_la_SOURCES = PluginTrayIcon.cc

trayicon_la_CXXFLAGS = -I$(top_srcdir)/lib $(GTKMM_CFLAGS) $(LIBTORRENT_CFLAGS) $(DBUSGLIB_CFLAGS) $(LIBGLADE_CFLAGS) ${GCONFMM_CFLAGS}

trayicon_la_CFLAGS = -I$(top_srcdir)/lib $(GTKMM_CFLAGS) $(DBUSGLIB_CFLAGS)

trayicon_la_LDFLAGS = -avoid-version -module -export-dynamic $(GTKMM_LIBS) $(LIBTORRENT_LIBS) $(DBUSGLIB_LIBS) $(LIBGLADE_LIBS) ${GCONFMM_LIBS} -L$(top_srcdir)/lib -llinkage-1
