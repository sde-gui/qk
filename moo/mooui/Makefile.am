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

noinst_LTLIBRARIES = libmooui.la

libmooui_la_SOURCES =           \
    mooaccel.c                  \
    mooaccel.h                  \
    mooaction.c                 \
    mooaction.h                 \
    mooactiongroup.c            \
    mooactiongroup.h            \
    moomenuaction.c             \
    moomenuaction.h             \
    mooshortcutsprefs.c         \
    mooshortcutsprefs.h         \
    mooshortcutsprefs-glade.h   \
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

BUILT_SOURCES = mooshortcutsprefs-glade.h

mooshortcutsprefs-glade.h: $(srcdir)/glade/shortcutsprefs.glade $(srcdir)/../mooutils/xml2h.sh
	sh $(srcdir)/../mooutils/xml2h.sh MOO_SHORTCUTS_PREFS_GLADE_UI \
	   $(srcdir)/glade/shortcutsprefs.glade > mooshortcutsprefs-glade.h
