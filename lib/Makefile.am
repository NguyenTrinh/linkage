SUBDIRS = linkage

lib_LTLIBRARIES = liblinkage-1.la

liblinkage_1_la_SOURCES = Engine.cc \
                          AlertManager.cc \
                          Plugin.cc \
                          PluginManager.cc \
                          SessionManager.cc \
                          SettingsManager.cc \
                          Torrent.cc \
                          TorrentAdaptor.cc \
                          TorrentManager.cc \
                          Utils.cc \
                          Interface.cc \
                          InterfaceAdaptor.cc

liblinkage_1_la_CXXFLAGS = $(CXXFLAGS) $(GTKMM_CFLAGS) \
	$(LIBTORRENT_CFLAGS) $(DBUSGLIB_CFLAGS) ${GCONFMM_CFLAGS}

liblinkage_1_la_LDFLAGS = -export-dynamic -version-info 0:5:0 $(LDFLAGS) \
	$(GTKMM_LIBS) $(LIBTORRENT_LIBS) $(DBUSGLIB_LIBS) ${GCONFMM_LIBS}

