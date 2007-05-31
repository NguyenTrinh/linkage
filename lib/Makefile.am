SUBDIRS = linkage

lib_LTLIBRARIES = liblinkage-1.la

plugindir = \"$(prefix)/share/linkage/plugins\"

if HAVE_KEYFILE
keyfile_src = 
else
keyfile_src = gtkmm_extra/keyfile.cc
endif

liblinkage_1_la_SOURCES = AlertManager.cc \
													DbusManager.cc \
													dbus-c.c \
													Engine.cc \
													Plugin.cc \
													PluginManager.cc \
													SessionManager.cc \
													SettingsManager.cc \
													Torrent.cc \
													TorrentManager.cc \
													Utils.cc $(keyfile_src)
													
liblinkage_1_la_CXXFLAGS = $(CXXFLAGS) $(GTKMM_CFLAGS) $(LIBTORRENT_CFLAGS) $(DBUSGLIB_CFLAGS) -DPLUGIN_DIR="$(plugindir)"

liblinkage_1_la_CFLAGS = $(CFLAGS) $(DBUSGLIB_CFLAGS)

liblinkage_1_la_LDFLAGS = -export-dynamic -version-info 0:3:0 $(LDFLAGS) $(GTKMM_LIBS) $(LIBTORRENT_LIBS) $(DBUSGLIB_LIBS)

if HAVE_KEYFILE
else
keyfileincludedir = $(includedir)/linkage/gtkmm_extra
keyfileinclude_HEADERS = gtkmm_extra/keyfile.h
keyfilepincludedir = $(includedir)/linkage/gtkmm_extra/private
keyfilepinclude_HEADERS = gtkmm_extra/private/keyfile_p.h
endif
