#
#   mooterm/Makefile.am
#

SUBDIRS = glade

widget_sources =                \
    mootermbuffer.c             \
    mootermbuffer-graph.h       \
    mootermbuffer.h             \
    mootermbuffer-private.h     \
    mooterm.c                   \
    mooterm-ctlfuncs.h          \
    mootermdraw.c               \
    mooterm.h                   \
    mooterminput.c              \
    mootermline.h               \
    mootermparser.c             \
    mootermparser.h             \
    mootermparser-yacc.y        \
    mooterm-private.h           \
    mootermpt.c                 \
    mootermpt.h                 \
    mootermpt-private.h         \
    mootermselection.c          \
    mooterm-vt.h

widget_unix_sources =       \
    mootermpt-unix.c        \
    pty.c                   \
    pty.h

widget_win32_sources =      \
    mootermpt-win32.c

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
AM_YFLAGS = -d

EXTRA_DIST =                \
    $(widget_sources)       \
    $(widget_unix_sources)  \
    $(widget_win32_sources)


############################################################################
#   UNIX
#
if UNIX_BUILD

noinst_LTLIBRARIES = libmooterm.la

libmooterm_la_SOURCES = $(widget_sources) $(widget_unix_sources)
BUILT_SOURCES = mootermparser-yacc.c mootermparser-yacc.h

endif UNIX_BUILD


############################################################################
#   MINGW
#
if MINGW_BUILD

noinst_LTLIBRARIES = libmooterm.la

libmooterm_la_SOURCES = $(widget_sources) $(widget_win32_sources)
BUILT_SOURCES = mootermparser-yacc.c mootermparser-yacc.h

endif MINGW_BUILD


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
