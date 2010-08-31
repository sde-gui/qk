noinst_LTLIBRARIES = libmooapp.la

EXTRA_DIST =
BUILT_SOURCES =

AM_CPPFLAGS = $(MOO_CPPFLAGS) -I.. -I$(top_srcdir)/doc
AM_CFLAGS = $(MOO_CFLAGS)
AM_CXXFLAGS = $(MOO_CXXFLAGS)

libmooapp_la_SOURCES =	\
	mooappabout.c	\
	mooappabout.h	\
	mooapp.c	\
	mooapp.h	\
	mooapp-accels.h	\
	mooapp-info.h	\
	mooapp-private.h\
	moohtml.h	\
	moohtml.c	\
	moolinklabel.h	\
	moolinklabel.c

EXTRA_DIST +=					\
	glade/mooappabout-dialog.glade		\
	glade/mooappabout-license.glade		\
	glade/mooappabout-credits.glade

BUILT_SOURCES +=				\
	glade/mooappabout-dialog-gxml.h		\
	glade/mooappabout-license-gxml.h	\
	glade/mooappabout-credits-gxml.h

BUILT_SOURCES += mooapp-credits.h
mooapp-credits.h: $(top_srcdir)/THANKS $(top_srcdir)/tools/xml2h.py
	$(PYTHON) $(top_srcdir)/tools/xml2h.py $(top_srcdir)/THANKS mooapp-credits.h MOO_APP_CREDITS

include ../../tools/rules.mk
