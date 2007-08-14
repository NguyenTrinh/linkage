upnpdir = $(libdir)/linkage/plugins

upnp_LTLIBRARIES = upnp.la

noinst_HEADERS = PluginUPnP.hh

upnp_la_SOURCES = PluginUPnP.cc

upnp_la_CXXFLAGS = -I$(top_srcdir)/lib $(GUPNP_CFLAGS) $(GTKMM_CFLAGS) $(LIBTORRENT_CFLAGS) $(DBUSGLIB_CFLAGS) $(GTHREAD_CFLAGS) $(LIBGLADE_CFLAGS) ${GCONFMM_CFLAGS}

upnp_la_LDFLAGS = -avoid-version -module -export-dynamic $(GUPNP_LIBS) $(GTKMM_LIBS) $(LIBTORRENT_LIBS) $(DBUSGLIB_CFLAGS) $(GTHREAD_LIBS) $(LIBGLADE_LIBS) ${GCONFMM_LIBS} -L$(top_srcdir)/lib -llinkage-1
