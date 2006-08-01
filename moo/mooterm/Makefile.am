mooterm_include_headers =	       \
	mooterm.h                      \
	mootermwindow.h                \
	mooterm-text.h                 \
	mootermline.h                  \
	mootermtag.h

mooterm_sources =                      \
	$(mooterm_include_headers)     \
	mooterm.c                      \
	mooterm-input.c                \
	mooterm-keymap.h               \
	mooterm-prefs.c                \
	mooterm-prefs.h                \
	mooterm-private.h              \
	mooterm-selection.h            \
	mooterm-text.c                 \
	mooterm-text-private.h         \
	mooterm-vt.h                   \
	mooterm-vtctls.h               \
	mooterm-draw.c                 \
	mootermbuffer.c                \
	mootermbuffer.h                \
	mootermbuffer-graph.h          \
	mootermbuffer-private.h        \
	mootermline.c                  \
	mootermline-private.h          \
	mootermparser.c                \
	mootermparser.h                \
	mootermparser-yacc.c           \
	mootermparser-yacc.h           \
	mootermpt.c                    \
	mootermpt.h                    \
	mootermpt-private.h            \
	mootermtag.c                   \
	mootermwindow.c                \
	mootermprefs-glade.h

mooterm_unix_sources =			\
	mootermpt-unix.c		\
	pty.c				\
	pty.h

mooterm_win32_sources =			\
	mootermhelper.h			\
	mootermpt-cygwin.c

termhelper_sources =			\
	termhelper_res.res		\
	mootermhelper.c			\
	mootermhelper.h			\
	pty.c				\
	pty.h

EXTRA_DIST =				\
	mootermparser-yacc.y		\
	$(termhelper_sources)		\
	termhelper_res.rc.in		\
	glade/mootermprefs.glade

BUILT_SOURCES =				\
	mootermprefs-glade.h

# not using rule for auto-regenerating mootermparser-yacc.[ch]
mootermparser_yacc: mootermparser-yacc.y
	touch mootermparser_yacc
	bison -o $(srcdir)/mootermparser-yacc.c -d $(srcdir)/mootermparser-yacc.y

XML2H = $(srcdir)/../mooutils/xml2h.sh
mootermprefs-glade.h: glade/mootermprefs.glade $(XML2H)
	sh $(XML2H) MOO_TERM_PREFS_GLADE_UI $(srcdir)/glade/mootermprefs.glade \
		> mootermprefs-glade.h


noinst_LTLIBRARIES = libmooterm.la

libmooterm_la_SOURCES = $(mooterm_sources)

if MOO_OS_UNIX
libmooterm_la_SOURCES += $(mooterm_unix_sources)
EXTRA_DIST += $(mooterm_win32_sources)
else
libmooterm_la_SOURCES += $(mooterm_win32_sources)
EXTRA_DIST += $(mooterm_unix_sources)
endif

if MOO_INSTALL_LIB
mooterm_includedir = $(moo_includedir)/mooterm
mooterm_include_HEADERS = $(mooterm_include_headers)
endif

AM_CFLAGS =			\
	-I$(top_builddir)	\
	-I..			\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)

if MOO_OS_CYGWIN
noinst_PROGRAMS = termhelper
termhelper_SOURCES = $(termhelper_sources)
nodist_termhelper_SOURCES = termhelper_res.res
CLEANFILES = termhelper_res.res
termhelper_CFLAGS =                         \
    -I$(top_builddir)                       \
    -I$(top_srcdir)                         \
    -I$(top_srcdir)/$(MOO_SRC_PREFIX)       \
    -I$(top_builddir)/$(MOO_SRC_PREFIX)     \
    $(MOO_DEBUG_CFLAGS)                     \
    $(CFLAGS)
termhelper_res.res: termhelper_res.rc
	$(WINDRES) -i $(srcdir)/termhelper_res.rc --input-format=rc -o termhelper_res.res -O coff
termhelper_LDADD = termhelper_res.res
else
termhelper_res.res: termhelper_res.rc
	touch termhelper_res.res
CLEANFILES = termhelper_res.res
endif
