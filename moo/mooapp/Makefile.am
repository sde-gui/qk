#
#   mooapp/Makefile.am
#

SUBDIRS = glade

DEFAULT_INCLUDES =
AM_CFLAGS =                             \
    -I$(top_builddir)                   \
    -I$(top_builddir)/$(MOO_SRC_PREFIX) \
    $(PYTHON_INCLUDES)                  \
    $(PYGTK_CFLAGS)                     \
    $(MOO_CFLAGS)                       \
    $(MOO_DEBUG_CFLAGS)                 \
    $(CFLAGS)

noinst_LTLIBRARIES   = libmooapp.la

BUILT_SOURCES = mooappabout-glade.h

libmooapp_la_SOURCES =      \
    mooappabout-glade.h     \
    mooappdialogs.c         \
    mooapp-prefs.c          \
    mooapp-private.h        \
    mooapp.c                \
    mooapp.h                \
    mooappinput.c           \
    mooappinput.h           \
    mooappoutput.c          \
    mooappoutput.h

if MOO_USE_PYTHON
libmooapp_la_SOURCES += \
    mooapp-python.h     \
    moopythonconsole.c  \
    moopythonconsole.h  \
    moopython.c         \
    moopython.h
endif MOO_USE_PYTHON


EXTRA_DIST =            \
    gpl                 \
    moopython.c         \
    moopython.h         \
    moopythonconsole.c  \
    moopythonconsole.h

mooappincludedir = $(includedir)/moo-1.0/mooapp
mooappinclude_HEADERS = mooapp.h


GLADE = $(srcdir)/glade
XML2SH = $(srcdir)/../mooutils/xml2h.sh
SUPPORT = $(top_srcdir)/$(MOO_SRC_PREFIX)/mooutils/glade/support.sh
mooappabout-glade.h: $(GLADE)/mooappabout.glade
	sh $(XML2SH) MOO_APP_ABOUT_GLADE_UI $(GLADE)/mooappabout.glade > mooappabout-glade.h
