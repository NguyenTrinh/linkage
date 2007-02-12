bin_PROGRAMS = linkage

flagdir = \"$(prefix)/share/linkage/flags\"

AM_CXXFLAGS = -I../lib $(CPPFLAGS) $(CXXFLAGS) $(GTKMM_CFLAGS) $(LIBTORRENT_CFLAGS)  $(LIBCURL_CFLAGS) $(DBUSGLIB_CFLAGS) -DFLAG_DIR="$(flagdir)"

linkage_SOURCES = CellRendererPieceMap.hh \
                  FileList.hh \
                  PeerList.hh \
                  PieceMap.hh \
                  SettingsWin.hh \
                  TorrentCreator.hh \
                  TorrentList.hh \
                  UI.hh \
                  CellRendererPieceMap.cc \
                  FileList.cc \
                  PeerList.cc \
                  PieceMap.cc \
                  SettingsWin.cc \
                  TorrentCreator.cc \
                  TorrentList.cc \
                  UI.cc \
                  GroupFilter.cc \
                  GroupFilter.hh \
                  AlignedLabel.hh \
                  AlignedLabel.cc \
                  AlignedSpinButton.hh \
                  AlignedSpinButton.cc \
                  Statusbar.hh \
                  Statusbar.cc \
                  CellRendererProgressText.hh \
                  CellRendererProgressText.cc \
                  main.cc

                  
linkage_LDADD = $(LDFLAGS) $(GTKMM_LIBS) $(LIBTORRENT_LIBS) $(LIBCURL_LIBS) $(DBUSGLIB_LIBS) -L../lib -llinkage-1
