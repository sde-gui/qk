#
#   mooterm/Makefile.am
#

SUBDIRS = glade

widget_sources =                \
    mooterm.c                   \
    mooterm.h                   \
    mooterm-input.c             \
    mooterm-keymap.h            \
    mooterm-prefs.c             \
    mooterm-prefs.h             \
    mooterm-private.h           \
    mooterm-selection.h         \
    mooterm-text.c              \
    mooterm-vt.h                \
    mooterm-vtctls.h            \
    mooterm-draw.c              \
    mootermbuffer.c             \
    mootermbuffer-graph.h       \
    mootermbuffer.h             \
    mootermbuffer-private.h     \
    mootermline.c               \
    mootermline.h               \
    mootermparser.c             \
    mootermparser.h             \
    mootermparser-yacc.y        \
    mootermpt.c                 \
    mootermpt.h                 \
    mootermpt-private.h         \
    mootermwindow.c             \
    mootermwindow.h

widget_unix_sources =       \
    mootermpt-unix.c        \
    pty.c                   \
    pty.h

widget_win32_sources =      \
    mootermhelper.h         \
    mootermpt-win32.c

termhelper_sources =        \
    mootermhelper.c         \
    mootermhelper.h         \
    pty.c                   \
    pty.h

AM_INCLUDES =
common_cflags =                         \
    -I$(top_builddir)                   \
    -I$(top_builddir)/$(MOO_SRC_PREFIX) \
    $(MOO_CFLAGS)                       \
    $(GTK_CFLAGS)                       \
    -DG_LOG_DOMAIN=\"Moo\"              \
    -D__MOO__

AM_CFLAGS = $(common_cflags) $(DEBUG_CFLAGS) $(CFLAGS)
AM_YFLAGS = -d

EXTRA_DIST =                \
    $(widget_sources)       \
    $(widget_unix_sources)  \
    $(widget_win32_sources) \
    $(termhelper_sources)   \
    termhelper_res.rc.in

noinst_LTLIBRARIES = libmooterm.la
libmooterm_la_SOURCES =
BUILT_SOURCES =


if UNIX_BUILD
libmooterm_la_SOURCES += $(widget_sources) $(widget_unix_sources) mootermprefs-glade.c
BUILT_SOURCES += mootermparser-yacc.c mootermparser-yacc.h mootermprefs-glade.c
endif UNIX_BUILD


if MINGW_BUILD
libmooterm_la_SOURCES += $(widget_sources) $(widget_win32_sources) mootermprefs-glade.c
BUILT_SOURCES += mootermparser-yacc.c mootermparser-yacc.h mootermprefs-glade.c
endif MINGW_BUILD


GLADE = $(srcdir)/glade
TOOLBAR = $(top_srcdir)/$(MOO_SRC_PREFIX)/mooutils/glade/toolbar.sh
SUPPORT = $(top_srcdir)/$(MOO_SRC_PREFIX)/mooutils/glade/support.sh

mootermprefs-glade.c: $(GLADE)/mootermprefs.c $(GLADE)/mootermprefs.sed $(TOOLBAR) $(SUPPORT)
	cat $(GLADE)/mootermprefs.c | sh $(SUPPORT) | sed -f $(GLADE)/mootermprefs.sed \
		> mootermprefs-glade.c


############################################################################
#   CYGWIN
#
if CYGWIN_BUILD

noinst_PROGRAMS = termhelper

termhelper_CFLAGS = \
    $(AM_CFLAGS)    \
    $(CYGWIN_CFLAGS)

termhelper_SOURCES = $(termhelper_sources)
termhelper_LDADD = $(CYGWIN_LDFLAGS) termhelper_res.res

termhelper_res.res: termhelper_res.rc
	$(WINDRES) -i termhelper_res.rc --input-format=rc -o termhelper_res.res -O coff

endif CYGWIN_BUILD
