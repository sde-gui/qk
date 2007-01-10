DIST_SUBDIRS = activestrings fileselector

# subdirs = activestrings completion
# libs = activestrings/libastrings.la completion/libcompletion.la
subdirs =
libs =
if GTK_2_6
subdirs += fileselector
libs += fileselector/libfileselector.la
endif
SUBDIRS = $(subdirs) .

BUILT_SOURCES =			\
	moofind-glade.h

EXTRA_DIST =			\
	moofind.glade

XML2H = $(srcdir)/../../mooutils/xml2h.sh
moofind-glade.h: moofind.glade $(XML2H)
	$(SHELL) $(XML2H) MOO_FIND_GLADE_XML $(srcdir)/moofind.glade > $@.tmp && \
	mv $@.tmp $@

unix_sources =			\
	moofind.c		\
	moofind-glade.h

noinst_LTLIBRARIES = libmooeditplugins.la
libmooeditplugins_la_LIBADD = $(libs)
libmooeditplugins_la_SOURCES =	\
	mooeditplugins.h

if MOO_OS_UNIX
libmooeditplugins_la_SOURCES += $(unix_sources)
else
EXTRA_DIST += $(unix_sources)
endif

AM_CFLAGS =			\
	-I../..			\
	-I$(top_builddir)	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)
