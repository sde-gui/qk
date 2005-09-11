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
    $(DEBUG_CFLAGS)                     \
    $(CFLAGS)                           \
    -DG_LOG_DOMAIN=\"Moo\"              \
    -D__MOO__

noinst_LTLIBRARIES   = libmooapp.la

BUILT_SOURCES =
libmooapp_la_SOURCES =      \
    mooappdialogs.c         \
    mooapp-prefs.c          \
    mooapp-private.h        \
    mooapp.c                \
    mooapp.h

if USE_PYTHON
libmooapp_la_SOURCES += \
    mooapp-python.h     \
    mooappinput.c       \
    mooappinput.h       \
    moopythonconsole.c  \
    moopythonconsole.h  \
    moopython.c         \
    moopython.h
endif USE_PYTHON


EXTRA_DIST =            \
    gpl                 \
    mooappinput.c       \
    mooappinput.h       \
    moopython.c         \
    moopython.h         \
    moopythonconsole.c  \
    moopythonconsole.h


if !GTK_2_6
libmooapp_la_SOURCES += mooappabout-glade.h
BUILT_SOURCES += mooappabout-glade.h
GLADE = $(srcdir)/glade
XML2SH = $(srcdir)/../mooutils/xml2h.sh
SUPPORT = $(top_srcdir)/$(MOO_SRC_PREFIX)/mooutils/glade/support.sh
mooappabout-glade.h: $(GLADE)/mooappabout.glade
	sh $(XML2SH) MOO_APP_ABOUT_GLADE_UI $(GLADE)/mooappabout.glade > mooappabout-glade.h
endif
