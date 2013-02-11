# $Id$

INCLUDES =									\
	-I$(top_builddir)						\
	-I$(top_srcdir)							\
	-DG_LOG_DOMAIN=\"thunar-nilfs2-plugin\"	\
	-DEXO_API_SUBJECT_TO_CHANGE				\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"	\
	$(PLATFORM_CPPFLAGS)

extensionsdir = $(libdir)/thunarx-2
extensions_LTLIBRARIES = libthunar-nilfs2-plugin.la

libthunar_nilfs2_plugin_la_SOURCES = nilfs2-plugin.c \
	nilfs2-provider.c nilfs2-page.c

libthunar_nilfs2_plugin_la_CFLAGS =	\
	$(EXO_CFLAGS)					\
	$(PLATFORM_CFLAGS)				\
	$(THUNARX_CFLAGS)

libthunar_nilfs2_plugin_la_LDFLAGS =	\
	-avoid-version						\
	-export-dynamic						\
	-module								\
	$(EXO_LIBS)							\
	$(THUNARX_LIBS)						\
	$(PLATFORM_LDFLAGS)					\
	-lnilfs
