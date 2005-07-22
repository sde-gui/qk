#
#   mooterm/Makefile.am
#

SUBDIRS = glade

widget_sources =                \
    mooterm.c                   \
    mooterm.h                   \
    mooterm-ctlfuncs.h          \
    mooterm-input.c             \
    mooterm-keymap.h            \
    mooterm-private.h           \
    mooterm-selection.h         \
    mooterm-text.c              \
    mooterm-vt.h                \
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
    mootermpt-private.h

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

noinst_LTLIBRARIES = libmooterm.la
libmooterm_la_SOURCES =
BUILT_SOURCES =


if UNIX_BUILD
libmooterm_la_SOURCES += $(widget_sources) $(widget_unix_sources)
BUILT_SOURCES += mootermparser-yacc.c mootermparser-yacc.h
endif UNIX_BUILD


if MINGW_BUILD
libmooterm_la_SOURCES += $(widget_sources) $(widget_win32_sources)
BUILT_SOURCES += mootermparser-yacc.c mootermparser-yacc.h
endif MINGW_BUILD


##############################################################################
# Pygtk stuff
#

EXTRA_DIST += mooterm-mod.c mooterm-pygtk.defs mooterm-pygtk.override

if USE_PYGTK
libmooterm_la_SOURCES += mooterm-mod.c

nodist_libmooterm_la_SOURCES = mooterm-pygtk.c
BUILT_SOURCES += mooterm-pygtk.c
CLEANFILES = mooterm-pygtk.c

mooterm-pygtk.c: $(srcdir)/mooterm-pygtk.defs $(srcdir)/mooterm-pygtk.override
	pygtk-codegen-2.0 --prefix moo_term \
	--register $(PYGTK_DEFS_DIR)/gtk-types.defs \
	--register $(PYGTK_DEFS_DIR)/gdk-types.defs \
	--register $(srcdir)/../mooui/mooui-pygtk.defs \
	--override $(srcdir)/mooterm-pygtk.override \
	$(srcdir)/mooterm-pygtk.defs > mooterm-pygtk.c

endif USE_PYGTK


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
