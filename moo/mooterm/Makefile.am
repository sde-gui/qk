#
#   mooterm/Makefile.am
#

SUBDIRS = glade

mootermincludedir = $(includedir)/moo-1.0/mooterm
mooterminclude_HEADERS =        \
    mooterm.h                   \
    mootermwindow.h             \
    mooterm-text.h              \
    mootermline.h               \
    mootermtag.h

widget_sources =                \
    $(mooterminclude_HEADERS)   \
    mooterm.c                   \
    mooterm-input.c             \
    mooterm-keymap.h            \
    mooterm-prefs.c             \
    mooterm-prefs.h             \
    mooterm-private.h           \
    mooterm-selection.h         \
    mooterm-text.c              \
    mooterm-text-private.h      \
    mooterm-vt.h                \
    mooterm-vtctls.h            \
    mooterm-draw.c              \
    mootermbuffer.c             \
    mootermbuffer.h             \
    mootermbuffer-graph.h       \
    mootermbuffer-private.h     \
    mootermline.c               \
    mootermline-private.h       \
    mootermparser.c             \
    mootermparser.h             \
    mootermparser-yacc.c        \
    mootermparser-yacc.h        \
    mootermpt.c                 \
    mootermpt.h                 \
    mootermpt-private.h         \
    mootermtag.c                \
    mootermwindow.c             \
    mootermprefs-glade.h

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
    $(GTK_CFLAGS)

AM_CFLAGS = $(common_cflags) $(DEBUG_CFLAGS) $(CFLAGS)

EXTRA_DIST =                \
    mootermparser-yacc.y    \
    $(widget_sources)       \
    $(widget_unix_sources)  \
    $(widget_win32_sources) \
    $(termhelper_sources)   \
    termhelper_res.rc.in

noinst_LTLIBRARIES = libmooterm.la
libmooterm_la_SOURCES =
BUILT_SOURCES =


if MOO_OS_UNIX
libmooterm_la_SOURCES += $(widget_sources) $(widget_unix_sources) mootermprefs-glade.h
BUILT_SOURCES += mootermprefs-glade.h
endif MOO_OS_UNIX


if MOO_OS_MINGW
libmooterm_la_SOURCES += $(widget_sources) $(widget_win32_sources) mootermprefs-glade.h
BUILT_SOURCES +=  mootermprefs-glade.h
endif MOO_OS_MINGW

# XXX this should be removed too
mootermparser-yacc.c: mootermparser-yacc.y
	bison -o $(srcdir)/mootermparser-yacc.c -d $(srcdir)/mootermparser-yacc.y


GLADE = $(srcdir)/glade
XML2H = $(srcdir)/../mooutils/xml2h.sh
mootermprefs-glade.h: $(GLADE)/mootermprefs.glade
	sh $(XML2H) MOO_TERM_PREFS_GLADE_UI $(GLADE)/mootermprefs.glade \
		> mootermprefs-glade.h


############################################################################
#   CYGWIN
#
if MOO_OS_CYGWIN

noinst_PROGRAMS = termhelper

termhelper_CFLAGS = \
    $(AM_CFLAGS)    \
    $(CYGWIN_CFLAGS)

termhelper_SOURCES = $(termhelper_sources)
termhelper_LDADD = $(CYGWIN_LDFLAGS) termhelper_res.res

termhelper_res.res: termhelper_res.rc
	$(WINDRES) -i termhelper_res.rc --input-format=rc -o termhelper_res.res -O coff

endif MOO_OS_CYGWIN
