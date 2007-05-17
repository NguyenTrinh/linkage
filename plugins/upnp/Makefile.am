upnpdir = $(prefix)/share/linkage/plugins

upnp_LTLIBRARIES = upnp.la

noinst_HEADERS = PluginUPnP.hh UPnPManager.hh

upnp_la_SOURCES = PluginUPnP.cc UPnPManager.cc

upnp_la_CXXFLAGS = -I../../lib $(LIBUPNP_CFLAGS) $(GTKMM_CFLAGS) $(LIBTORRENT_CFLAGS) $(DBUSGLIB_CFLAGS) $(GTHREAD_CFLAGS) $(LIBGLADE_CFLAGS)

upnp_la_LDFLAGS = -avoid-version -module -export-dynamic $(LIBUPNP_LIBS) $(GTKMM_LIBS) $(LIBTORRENT_LIBS) $(DBUSGLIB_LIBS) $(GTHREAD_LIBS) $(LIBGLADE_LIBS) -L../../lib -llinkage-1
