schedulerdir = $(libdir)/linkage/plugins

scheduler_LTLIBRARIES = scheduler.la

noinst_HEADERS = PluginScheduler.hh SchedulerConfig.hh ScheduleWin.hh

scheduler_la_SOURCES = PluginScheduler.cc \
                       SchedulerConfig.hh \
                       ScheduleWin.cc 

scheduler_la_CXXFLAGS = -I$(top_srcdir)/lib $(LIBNOTIFY_CFLAGS) $(GTKMM_CFLAGS) $(LIBTORRENT_CFLAGS) $(LIBGLADE_CFLAGS) ${GCONFMM_CFLAGS} $(EXO_CFLAGS) $(GNOME_CFLAGS) $(DBUSGLIB_CFLAGS)

scheduler_la_LDFLAGS = -avoid-version -module -export-dynamic $(LIBNOTIFY_LIBS) $(GTKMM_LIBS) $(LIBTORRENT_LIBS) $(LIBGLADE_LIBS) ${GCONFMM_LIBS} $(GNOME_LIBS) $(EXO_LIBS) $(DBUSGLIB_LIBS) -L$(top_srcdir)/lib -llinkage-1

imagedir = ${prefix}/share/linkage
image_DATA = normal.png limit1.png limit2.png limit3.png blocked.png

gladedir = ${prefix}/share/linkage
glade_DATA = scheduler.glade
