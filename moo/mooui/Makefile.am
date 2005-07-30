#
#   mooui/Makefile.am
#

SUBDIRS = glade .

DEFAULT_INCLUDES =
AM_CFLAGS =                             \
    -I$(top_builddir)                   \
    -I$(top_builddir)/$(MOO_SRC_PREFIX) \
    $(MOO_CFLAGS)                       \
    $(DEBUG_CFLAGS)                     \
    $(CFLAGS)                           \
    $(XML_CFLAGS)                       \
    -DG_LOG_DOMAIN=\"Moo\"

noinst_LTLIBRARIES   = libmooui.la

libmooui_la_SOURCES =           \
    mooaccel.c                  \
    mooaccel.h                  \
    mooaction.c                 \
    mooaction.h                 \
    mooactiongroup.c            \
    mooactiongroup.h            \
    moomenuaction.c             \
    moomenuaction.h             \
    mooshortcutsprefs-glade.c   \
    mooshortcutsprefs.c         \
    mooshortcutsprefs.h         \
    mootext.c                   \
    mootext.h                   \
    mootoggleaction.c           \
    mootoggleaction.h           \
    moouiobject.c               \
    moouiobject.h               \
    moouiobject-impl.h          \
    moouixml.c                  \
    moouixml.h                  \
    moowindow.c                 \
    moowindow.h

EXTRA_DIST =                    \
    moouimanager.cpp            \
    moouimanager.h

BUILT_SOURCES = mooshortcutsprefs-glade.c

GLADE = $(srcdir)/glade
SUPPORT = $(top_srcdir)/$(MOO_SRC_PREFIX)/mooutils/glade/support.sh
mooshortcutsprefs-glade.c: $(GLADE)/shortcutsprefs.c $(GLADE)/shortcutsprefs.sed $(SUPPORT)
	cat $(GLADE)/shortcutsprefs.c | sh $(SUPPORT) | \
		sed -f $(GLADE)/shortcutsprefs.sed > mooshortcutsprefs-glade.c
