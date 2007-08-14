notificationdir = $(libdir)/linkage/plugins

notification_LTLIBRARIES = notification.la

noinst_HEADERS = PluginNotification.hh

notification_la_SOURCES = PluginNotification.cc

notification_la_CXXFLAGS = -I$(top_srcdir)/lib $(LIBNOTIFY_CFLAGS) $(GTKMM_CFLAGS) $(LIBTORRENT_CFLAGS) $(LIBGLADE_CFLAGS) ${GCONFMM_CFLAGS}

notification_la_LDFLAGS = -avoid-version -module -export-dynamic $(LIBNOTIFY_LIBS) $(GTKMM_LIBS) $(LIBTORRENT_LIBS) $(LIBGLADE_LIBS) ${GCONFMM_LIBS} -L$(top_srcdir)/lib -llinkage-1
