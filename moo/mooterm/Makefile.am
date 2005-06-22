#
#   mooterm/Makefile.am
#

SUBDIRS = glade

widget_sources =            \
    mooterm.c               \
    mooterm.h               \
    mooterm-private.h       \
    mootermbuffer.c         \
    mootermbuffer.h         \
    mootermbuffer-private.h \
    mootermcursor.c         \
    mootermdraw.c           \
    mootermline.h           \
    mootermparser.c         \
    mootermparser.h         \
    mootermselection.c      \
    mootermvt.c             \
    mootermvt.h

AM_INCLUDES =
common_cflags =                         \
    -I$(top_builddir)                   \
    -I$(top_builddir)/$(MOO_SRC_PREFIX) \
    $(MOO_CFLAGS)                       \
    $(PYTHON_INCLUDES)                  \
    $(PYGTK_CFLAGS)                     \
    $(GTK_CFLAGS)                       \
    -DG_LOG_DOMAIN=\"Moo\"
AM_CFLAGS = $(common_cflags) $(M_CFLAGS) $(CFLAGS)
AM_CXXFLAGS = $(common_cflags) $(M_CXXFLAGS) $(CXXFLAGS)

noinst_LTLIBRARIES = libmooterm.la

libmooterm_la_SOURCES = $(widget_sources)
