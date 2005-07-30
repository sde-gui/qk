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
    -DG_LOG_DOMAIN=\"Moo\"

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
libmooapp_la_SOURCES += mooappabout-glade.c
BUILT_SOURCES += mooappabout-glade.c
GLADE = $(srcdir)/glade
SUPPORT = $(top_srcdir)/$(MOO_SRC_PREFIX)/mooutils/glade/support.sh
mooappabout-glade.c: $(GLADE)/mooappabout.c $(GLADE)/mooappabout.sed $(SUPPORT)
	cat $(GLADE)/mooappabout.c | sh $(SUPPORT) | sed -f $(GLADE)/mooappabout.sed \
		> mooappabout-glade.c
endif
