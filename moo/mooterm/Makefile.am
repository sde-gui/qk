mooterm_sources =			\
	mooterm-draw.c			\
	mooterm-input.c			\
	mooterm-keymap.h		\
	mooterm-prefs.c			\
	mooterm-prefs.h			\
	mooterm-private.h		\
	mooterm-selection.h		\
	mooterm-text.c			\
	mooterm-text.h			\
	mooterm-vt.h			\
	mooterm-vtctls.h		\
	mooterm.c			\
	mooterm.h			\
	mootermbuffer-graph.h		\
	mootermbuffer-private.h		\
	mootermbuffer.c			\
	mootermbuffer.h			\
	mootermline-private.h		\
	mootermline.c			\
	mootermline.h			\
	mootermparser-yacc.c		\
	mootermparser-yacc.h		\
	mootermparser.c			\
	mootermparser.h			\
	mootermpt-private.h		\
	mootermpt.c			\
	mootermpt.h			\
	mootermtag-private.h		\
	mootermtag.c			\
	mootermtag.h

mooterm_unix_sources =			\
	mootermpt-unix.c		\
	pty.c				\
	pty.h

mooterm_win32_sources =			\
	mootermhelper.h			\
	mootermpt-cygwin.c

termhelper_sources =			\
	mootermhelper_res.res		\
	mootermhelper.c			\
	mootermhelper.h			\
	pty.c				\
	pty.h

EXTRA_DIST =				\
	mootermparser-yacc.y		\
	$(termhelper_sources)		\
	mootermhelper_res.rc.in

# not using rule for auto-regenerating mootermparser-yacc.[ch]
mootermparser_yacc: mootermparser-yacc.y
	touch mootermparser_yacc
	bison -o $(srcdir)/mootermparser-yacc.c -d $(srcdir)/mootermparser-yacc.y

BUILT_SOURCES =
CLEANFILES =

glade_files = glade/mootermprefs.glade
include ../../mk/glade.mk

noinst_LTLIBRARIES =

if !MOO_OS_CYGWIN
noinst_LTLIBRARIES += libmooterm.la
endif

libmooterm_la_SOURCES = $(mooterm_sources)
nodist_libmooterm_la_SOURCES = $(nodist_mooterm_sources)

if MOO_OS_UNIX
libmooterm_la_SOURCES += $(mooterm_unix_sources)
EXTRA_DIST += $(mooterm_win32_sources)
else
libmooterm_la_SOURCES += $(mooterm_win32_sources)
EXTRA_DIST += $(mooterm_unix_sources)
endif

AM_CFLAGS =			\
	-I$(top_builddir)	\
	-I..			\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)

if MOO_OS_CYGWIN
noinst_PROGRAMS = termhelper
termhelper_SOURCES = $(termhelper_sources)
nodist_termhelper_SOURCES = mootermhelper_res.res
CLEANFILES += mootermhelper_res.res
termhelper_CFLAGS =                         \
    -I$(top_builddir)                       \
    -I$(top_srcdir)                         \
    -I$(top_srcdir)/$(MOO_SRC_PREFIX)       \
    -I$(top_builddir)/$(MOO_SRC_PREFIX)     \
    $(MOO_DEBUG_CFLAGS)                     \
    $(CFLAGS)
mootermhelper_res.res: mootermhelper_res.rc
	$(WINDRES) -i mootermhelper_res.rc --input-format=rc -o mootermhelper_res.res -O coff
termhelper_LDADD = mootermhelper_res.res
else
mootermhelper_res.res: mootermhelper_res.rc
	touch mootermhelper_res.res
CLEANFILES += mootermhelper_res.res
endif

include ../../mk/marshals.mk
