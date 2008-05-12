AM_CFLAGS =			\
	-I../..			\
	-I$(top_builddir)	\
	-I$(top_builddir)/moo	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)
AM_OBJCFLAGS = $(AM_CFLAGS)

noinst_LTLIBRARIES = libproject.la

# EXTRA_DIST +=				\
# 	moofileselector.glade		\
# 	moofileselector-prefs.glade

libproject_la_SOURCES =			\
	project-plugin.m		\
	manager.h			\
	manager.m			\
	mpconfig.h			\
	mpconfig.m			\
	mpfile.h			\
	mpfile.m			\
	project.h			\
	project.m

# nodist_libfileselector_la_SOURCES =	\
# 	moofileselector-glade.h		\
# 	moofileselector-prefs-glade.h

# BUILT_SOURCES +=			\
# 	moofileselector-glade.h		\
# 	moofileselector-prefs-glade.h
# CLEANFILES +=				\
# 	moofileselector-glade.h		\
# 	moofileselector-prefs-glade.h
