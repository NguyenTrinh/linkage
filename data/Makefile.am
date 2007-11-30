SUBDIRS = flags

EXTRA_DIST = linkage.desktop.in linkage.svg linkage.glade linkage.schemas.in

desktopdir = $(prefix)/share/applications
desktop_in_files = linkage.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

pixmapdir = $(prefix)/share/pixmaps
pixmap_DATA = linkage.svg

gladedir = ${prefix}/share/linkage
glade_DATA = linkage.glade

schemadir = $(GCONF_SCHEMA_FILE_DIR)
schema_in_files = linkage.schemas.in
schema_DATA = $(schema_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@ 

if GCONF_SCHEMAS_INSTALL
install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA)
endif

