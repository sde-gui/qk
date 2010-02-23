completiondir = $(MOO_DATA_DIR)/completion
completion_DATA =		\
	files/gap.lst		\
	files/latex.cfg


EXTRA_DIST =			\
	$(completion_DATA)	\
	completion.glade	\
	files/texinfo.cfg

completion_sources =		\
	completion-glade.h	\
	completion-plugin.c	\
	completion.c		\
	completion.h

BUILT_SOURCES =			\
	completion-glade.h

completion-glade.h: completion.glade $(XML2H)
	$(SHELL) $(XML2H) COMPLETION_PLUGIN_GLADE_XML $(srcdir)/completion.glade > $@.tmp && \
	mv $@.tmp $@

noinst_LTLIBRARIES = libcompletion.la
libcompletion_la_SOURCES = $(completion_sources)

AM_CFLAGS =			\
	-I../../..		\
	-I$(top_builddir)	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)
