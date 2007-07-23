bin_PROGRAMS = linkage

AM_CXXFLAGS = -I../lib $(CPPFLAGS) $(CXXFLAGS) $(GTKMM_CFLAGS) $(LIBTORRENT_CFLAGS)  $(LIBCURL_CFLAGS) $(DBUSGLIB_CFLAGS) $(GTHREAD_CFLAGS) ${LIBGLADE_CFLAGS} ${GCONFMM_CFLAGS} $(EXO_CFLAGS) $(GNOME_CFLAGS) -DLOCALEDIR=\""$(datadir)/locale"\"

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
                  Statusbar.hh \
                  Statusbar.cc \
                  CellRendererProgressText.hh \
                  CellRendererProgressText.cc \
                  GroupList.hh \
                  GroupList.cc \
                  GroupsWin.hh \
                  GroupsWin.cc \
                  GroupEditDialog.hh \
                  GroupEditDialog.cc \
                  FilterRow.hh \
                  FilterRow.cc \
                  TorrentMenu.hh \
                  TorrentMenu.cc \
                  StateFilter.hh \
                  StateFilter.cc \
                  main.cc


linkage_LDADD = $(LDFLAGS) $(GTKMM_LIBS) $(LIBTORRENT_LIBS) \
	 $(LIBCURL_LIBS) $(DBUSGLIB_LIBS) $(GTHREAD_LIBS) \
	 ${LIBGLADE_LIBS} ${GCONFMM_LIBS} $(GNOME_LIBS) $(EXO_LIBS) -L../lib -llinkage-1
