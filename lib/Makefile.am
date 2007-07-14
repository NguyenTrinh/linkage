SUBDIRS = linkage

lib_LTLIBRARIES = liblinkage-1.la

plugindir = \"$(prefix)/share/linkage/plugins\"

liblinkage_1_la_SOURCES = Engine.cc \
                          AlertManager.cc \
                          Plugin.cc \
                          PluginManager.cc \
                          SessionManager.cc \
                          SettingsManager.cc \
                          Torrent.cc \
                          TorrentManager.cc \
                          DbusManager.cc \
                          dbus-c.c \
                          Utils.cc
                          
liblinkage_1_la_CXXFLAGS = $(CXXFLAGS) $(GTKMM_CFLAGS) \
	$(LIBTORRENT_CFLAGS) $(DBUSGLIB_CFLAGS) \
	${GCONFMM_CFLAGS} -DPLUGIN_DIR="$(plugindir)"

liblinkage_1_la_CFLAGS = $(CFLAGS) $(DBUSGLIB_CFLAGS)

liblinkage_1_la_LDFLAGS = -export-dynamic -version-info 0:3:0 $(LDFLAGS) \
	$(GTKMM_LIBS) $(LIBTORRENT_LIBS) $(DBUSGLIB_LIBS) ${GCONFMM_LIBS}

