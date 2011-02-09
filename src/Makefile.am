# gedit-smart-indenter
plugindir = $(GEDIT_PLUGINS_LIBS_DIR)
supportdir = $(GEDIT_PLUGINS_DATA_DIR)/smartindenter

INCLUDES = \
	-I$(top_srcdir) 						\
	$(GEDIT_CFLAGS) 						\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED_CFLAGS)

plugin_LTLIBRARIES = libsmartindenter.la

libsmartindenter_la_SOURCES = \
	gsi-plugin.h	\
	gsi-plugin.c	\
	gsi-indenter.h			\
	gsi-indenter.c			\
	gsi-indenter-utils.h		\
	gsi-indenter-utils.c		\
	gsi-indenters-manager.h		\
	gsi-indenters-manager.c		\
	gsi-indenter-simple.h		\
	gsi-indenter-simple.c		\
	gsi-indenter-c.h		\
	gsi-indenter-c.c		\
	gsi-indenter-python.h		\
	gsi-indenter-python.c

libsmartindenter_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libsmartindenter_la_LIBADD = $(GEDIT_LIBS)

support_DATA =

# Plugin Info
plugin_in_files = smartindenter.plugin.desktop.in

%.plugin: %.plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

BUILDFILES = $(plugin_in_files:.plugin.desktop.in=.plugin)

plugin_DATA = $(BUILDFILES)

EXTRA_DIST = $(plugin_in_files) $(support_DATA)

CLEANFILES = $(BUILDFILES)
DISTCLEANFILES = $($(support_DATA))

-include $(top_srcdir)/git.mk
