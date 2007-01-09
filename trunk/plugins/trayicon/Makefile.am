trayicondir = $(prefix)/share/linkage/plugins
pixmapdir = \"$(prefix)/share/pixmaps\"

trayicon_LTLIBRARIES = trayicon.la

noinst_HEADERS = PluginTrayIcon.hh eggtrayicon.h

trayicon_la_SOURCES = PluginTrayIcon.cc eggtrayicon.c

trayicon_la_CXXFLAGS = -I../../lib $(GTKMM_CFLAGS) $(LIBTORRENT_CFLAGS) $(DBUSGLIB_CFLAGS) -DPIXMAP_DIR="$(pixmapdir)"

trayicon_la_CFLAGS = -I../../lib $(GTKMM_CFLAGS) $(DBUSGLIB_CFLAGS)

trayicon_la_LDFLAGS = -avoid-version -module -export-dynamic $(GTKMM_LIBS) $(LIBTORRENT_LIBS) $(DBUSGLIB_LIBS) -L../../lib -llinkage-1
