pixmapdir = \"$(prefix)/share/pixmaps\"

notificationdir = $(prefix)/share/linkage/plugins

notification_LTLIBRARIES = notification.la

noinst_HEADERS = PluginNotification.hh

notification_la_SOURCES = PluginNotification.cc

notification_la_CXXFLAGS = -I../../lib $(LIBNOTIFY_CFLAGS) $(GTKMM_CFLAGS) $(LIBTORRENT_CFLAGS) $(LIBGLADE_CFLAGS) ${GCONFMM_CFLAGS}-DPIXMAP_DIR="$(pixmapdir)"

notification_la_LDFLAGS = -avoid-version -module -export-dynamic $(LIBNOTIFY_LIBS) $(GTKMM_LIBS) $(LIBTORRENT_LIBS) $(LIBGLADE_LIBS) ${GCONFMM_LIBS} -L../../lib -llinkage-1
