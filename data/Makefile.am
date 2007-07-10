SUBDIRS = flags

EXTRA_DIST = linkage.desktop linkage.svg linkage.glade settings.glade linkage.schemas.in

desktopdir = $(prefix)/share/applications
desktop_DATA = linkage.desktop

pixmapdir = $(prefix)/share/pixmaps
pixmap_DATA = linkage.svg

gladedir = ${prefix}/share/linkage
glade_DATA = linkage.glade settings.glade groups.glade

#
# Gconf schemas
#

schema_in_files = linkage.schemas.in
schemadir = $(GCONF_SCHEMA_FILE_DIR)
schema_DATA = $(schema_in_files:.schemas.in=.schemas)

if GCONF_SCHEMAS_INSTALL
# Install the gcalctool schema file.
install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA)
endif

