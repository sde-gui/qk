completiondir = $(MOO_DATA_DIR)/completion
completion_DATA =		\
	files/gap.lst		\
	files/latex.cfg


EXTRA_DIST =			\
	$(completion_DATA)	\
	files/texinfo.cfg

completion_sources =		\
	completion-plugin.c	\
	completion.c		\
	completion.h

noinst_LTLIBRARIES = libcompletion.la
libcompletion_la_SOURCES = $(completion_sources)

AM_CFLAGS =			\
	-I../../..		\
	-I$(top_builddir)	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)
