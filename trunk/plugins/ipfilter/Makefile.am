ipfilterdir = $(libdir)/linkage/plugins

ipfilter_LTLIBRARIES = ipfilter.la

noinst_HEADERS = IpFilterPlugin.hh

ipfilter_la_SOURCES = IpFilterPlugin.cc \
	libp2p/src/compact_list.cpp \
	libp2p/src/list.cpp \
	libp2p/src/list_p2b.cpp \
	libp2p/src/list_p2p.cpp \
	libp2p/src/stdafx.cpp


ipfilter_la_CXXFLAGS = -I$(top_srcdir)/lib $(GTKMM_CFLAGS) $(LIBTORRENT_CFLAGS) $(DBUSGLIB_CFLAGS) $(LIBGLADE_CFLAGS) ${GCONFMM_CFLAGS} -Ilibp2p/include -Ilibp2p/src

ipfilter_la_LDFLAGS = -avoid-version -module -export-dynamic $(GTKMM_LIBS) $(LIBTORRENT_LIBS) $(DBUSGLIB_LIBS) $(LIBGLADE_LIBS) ${GCONFMM_LIBS} -L$(top_srcdir)/lib -llinkage-1
