SUBDIRS = linkage

lib_LTLIBRARIES = liblinkage-1.la

plugindir = \"$(prefix)/share/linkage/plugins\"

liblinkage_1_la_SOURCES = gtkmm_extra/keyfile.cc \
                          gtkmm_extra/private/keyfile_p.h \
                          Engine.cc \
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
                          
liblinkage_1_la_CXXFLAGS = $(CXXFLAGS) $(GTKMM_CFLAGS) $(LIBTORRENT_CFLAGS) $(DBUSGLIB_CFLAGS) -DPLUGIN_DIR="$(plugindir)"

liblinkage_1_la_CFLAGS = $(CFLAGS) $(DBUSGLIB_CFLAGS)

liblinkage_1_la_LDFLAGS = -export-dynamic -version-info 0:1:0 $(LDFLAGS) $(GTKMM_LIBS) $(LIBTORRENT_LIBS) $(DBUSGLIB_LIBS)

keyfileincludedir = $(includedir)/linkage/gtkmm_extra
keyfileinclude_HEADERS = gtkmm_extra/keyfile.h
keyfilepincludedir = $(includedir)/linkage/gtkmm_extra/private
keyfilepinclude_HEADERS = gtkmm_extra/private/keyfile_p.h
