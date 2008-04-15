ipfilterdir = $(libdir)/linkage/plugins

ipfilter_LTLIBRARIES = ipfilter.la

noinst_HEADERS = IpFilterPlugin.hh \
	libp2p/include/p2p/compact_list.hpp \
	libp2p/include/p2p/exception.hpp \
	libp2p/include/p2p/ip.hpp \
	libp2p/include/p2p/list.hpp \
	libp2p/include/p2p/range.hpp


ipfilter_la_SOURCES = IpFilterPlugin.cc \
	libp2p/src/compact_list.cpp \
	libp2p/src/list.cpp \
	libp2p/src/list_p2b.cpp \
	libp2p/src/list_p2p.cpp \
	libp2p/src/stdafx.cpp \
	libp2p/src/stdafx.h \
	libp2p/src/utf8.h


ipfilter_la_CXXFLAGS = -I$(top_srcdir)/lib $(GTKMM_CFLAGS) $(LIBTORRENT_CFLAGS) $(DBUSGLIB_CFLAGS) $(LIBGLADE_CFLAGS) ${GCONFMM_CFLAGS} -I$(top_srcdir)/plugins/ipfilter/libp2p/include -I$(top_srcdir)/plugins/ipfilter/libp2p/src

ipfilter_la_LDFLAGS = -avoid-version -module -export-dynamic $(GTKMM_LIBS) $(LIBTORRENT_LIBS) $(DBUSGLIB_LIBS) $(LIBGLADE_LIBS) ${GCONFMM_LIBS} -L$(top_srcdir)/lib -llinkage-1
