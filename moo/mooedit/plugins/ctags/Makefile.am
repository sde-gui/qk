AM_CFLAGS =			\
	-I../..			\
	-I$(top_builddir)	\
	-I$(top_builddir)/moo	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)
AM_OBJCFLAGS = $(AM_CFLAGS)

noinst_LTLIBRARIES = libctags.la

# EXTRA_DIST +=				\
# 	moofileselector.glade		\
# 	moofileselector-prefs.glade

libctags_la_SOURCES =			\
	readtags.c			\
	readtags.h			\
	ctags-plugin.c			\
	ctags-doc.c			\
	ctags-doc.h			\
	ctags-view.c			\
	ctags-view.h

@MOO_GLADE_RULE@
# nodist_libfileselector_la_SOURCES =	\
# 	moofileselector-glade.h		\
# 	moofileselector-prefs-glade.h

# BUILT_SOURCES +=			\
# 	moofileselector-glade.h		\
# 	moofileselector-prefs-glade.h
# CLEANFILES +=				\
# 	moofileselector-glade.h		\
# 	moofileselector-prefs-glade.h
