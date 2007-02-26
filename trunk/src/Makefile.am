bin_PROGRAMS = linkage

flagdir = \"$(prefix)/share/linkage/flags\"
pixmapdir = \"$(prefix)/share/pixmaps\"

AM_CXXFLAGS = -I../lib $(CPPFLAGS) $(CXXFLAGS) $(GTKMM_CFLAGS) $(LIBTORRENT_CFLAGS)  $(LIBCURL_CFLAGS) $(DBUSGLIB_CFLAGS) -DFLAG_DIR="$(flagdir)" -DPIXMAP_DIR="$(pixmapdir)"

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
                  FilterManager.hh \
                  FilterManager.cc \
                  GroupFilterRow.hh \
                  GroupFilterRow.cc \
                  GroupFilterView.hh \
                  GroupFilterView.cc \
                  main.cc


linkage_LDADD = $(LDFLAGS) $(GTKMM_LIBS) $(LIBTORRENT_LIBS) $(LIBCURL_LIBS) $(DBUSGLIB_LIBS) -L../lib -llinkage-1
