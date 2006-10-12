SUBDIRS = linkage

lib_LTLIBRARIES = liblinkage-1.la

plugindir = \"$(prefix)/share/linkage/plugins\"

liblinkage_1_la_SOURCES = gtkmm_extra/keyfile.cc \
                          gtkmm_extra/private/keyfile_p.h \
                          AlertManager.hh \
                          DetailsIface.hh \
                          MainIface.hh \
                          Plugin.hh \
                          PluginManager.hh \
                          SessionManager.hh \
                          SettingsManager.hh \
                          Torrent.hh \
                          TorrentManager.hh \
                          TorrentMenuIface.hh \
                          Utils.hh \
                          AlertManager.cc \
                          DetailsIface.cc \
                          MainIface.cc \
                          Plugin.cc \
                          PluginManager.cc \
                          SessionManager.cc \
                          SettingsManager.cc \
                          Torrent.cc \
                          TorrentManager.cc \
                          TorrentMenuIface.cc \
                          Utils.cc
                          
liblinkage_1_la_CXXFLAGS = $(CPPFLAGS) $(CXXFLAGS) $(GTKMM_CFLAGS) $(LIBTORRENT_CFLAGS) -DPLUGIN_DIR="$(plugindir)"

liblinkage_1_la_LDFLAGS = -export-dynamic $(LDFLAGS) $(GTKMM_LIBS) $(LIBTORRENT_LIBS)

keyfileincludedir = $(includedir)/linkage/gtkmm_extra
keyfileinclude_HEADERS = gtkmm_extra/keyfile.h
keyfilepincludedir = $(includedir)/linkage/gtkmm_extra/private
keyfilepinclude_HEADERS = gtkmm_extra/private/keyfile_p.h
