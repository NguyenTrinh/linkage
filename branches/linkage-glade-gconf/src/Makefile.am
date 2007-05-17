bin_PROGRAMS = linkage

flagdir = \"$(prefix)/share/linkage/flags\"
pixmapdir = \"$(prefix)/share/pixmaps\"
datadir = \"${prefix}/share/linkage\"

AM_CXXFLAGS = -I../lib $(CPPFLAGS) $(CXXFLAGS) $(GTKMM_CFLAGS) $(LIBTORRENT_CFLAGS)  $(LIBCURL_CFLAGS) $(DBUSGLIB_CFLAGS) $(GTHREAD_CFLAGS) ${LIBGLADE_CFLAGS} -DFLAG_DIR="$(flagdir)" -DPIXMAP_DIR="$(pixmapdir)" -DDATA_DIR="${datadir}"

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
                  Group.cc \
                  Group.hh \
                  AlignedLabel.hh \
                  AlignedLabel.cc \
                  AlignedSpinButton.hh \
                  AlignedSpinButton.cc \
                  AlignedFrame.hh \
                  AlignedFrame.cc \
                  Statusbar.hh \
                  Statusbar.cc \
                  CellRendererProgressText.hh \
                  CellRendererProgressText.cc \
                  GroupList.hh \
                  GroupList.cc \
                  GroupRow.hh \
                  GroupRow.cc \
                  GroupView.hh \
                  GroupView.cc \
                  TorrentMenu.hh \
                  TorrentMenu.cc \
                  ExpanderButton.hh \
                  ExpanderButton.cc \
                  main.cc


linkage_LDADD = $(LDFLAGS) $(GTKMM_LIBS) $(LIBTORRENT_LIBS) $(LIBCURL_LIBS) $(DBUSGLIB_LIBS) $(GTHREAD_LIBS) ${LIBGLADE_LIBS} -L../lib -llinkage-1
