bin_PROGRAMS = linkage

flagdir = \"$(prefix)/share/linkage/flags\"

AM_CXXFLAGS = -I../lib $(CPPFLAGS) $(CXXFLAGS) $(GTKMM_CFLAGS) $(LIBTORRENT_CFLAGS) $(LIBCURL_CFLAGS) -DFLAG_DIR="$(flagdir)"

linkage_SOURCES = CellRendererPieceMap.hh \
                  FileList.hh \
                  IOManager.hh \
                  PeerList.hh \
                  PieceMap.hh \
                  SettingsWin.hh \
                  TorrentCreator.hh \
                  TorrentList.hh \
                  UI.hh \
                  CellRendererPieceMap.cc \
                  FileList.cc \
                  IOManager.cc \
                  PeerList.cc \
                  PieceMap.cc \
                  SettingsWin.cc \
                  TorrentCreator.cc \
                  TorrentList.cc \
                  UI.cc \
                  main.cc

                  
linkage_LDADD = $(LDFLAGS) $(GTKMM_LIBS) $(LIBTORRENT_LIBS) $(LIBCURL_LIBS) -L../lib -llinkage-1
