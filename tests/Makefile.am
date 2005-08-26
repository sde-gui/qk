#
#   tests/Makefile.am
#

EXTRA_PROGRAMS = medit mterm markup editor termbuffer testfileview testpaned testpanedfileview
bin_PROGRAMS =
noinst_PROGRAMS =

EXTRA_DIST =        \
    pyapp.py.in     \
    meditui.xml     \
    editor-ui.xml

BUILT_SOURCES = editor-ui.h
editor-ui.h: editor-ui.xml
	sh $(srcdir)/../moo/mooutils/xml2h.sh MEDIT_UI $(srcdir)/editor-ui.xml > editor-ui.h

all-am: pyapp.py editor-ui.h

if BUILD_MOOAPP
bin_PROGRAMS += editor
endif
if BUILD_MOOEDIT
noinst_PROGRAMS += medit testfileview testpanedfileview testpaned
endif
if BUILD_MOOTERM
noinst_PROGRAMS += mterm termbuffer
endif
if BUILD_MOOUTILS
noinst_PROGRAMS += markup
endif


AM_CFLAGS =                     \
    -I$(top_builddir)           \
    $(MOO_CFLAGS)               \
    $(DEBUG_CFLAGS)             \
    $(CFLAGS)                   \
    $(GTK_CFLAGS)


##############################################################################
##  editor
##
editor_LDFLAGS =
editor_LDADD =          \
    ../moo/libmoo.la    \
    $(MOO_LIBS)         \
    $(PYTHON_LDFLAGS)   \
    $(PYTHON_EXTRA_LIBS)


if MINGW_BUILD
editor_LDFLAGS += -mwindows
endif MINGW_BUILD

editor_SOURCES =    \
    editor.c        \
    editor-ui.h


##############################################################################
##  medit
##
medit_LDFLAGS =
medit_LDADD = $(MOO_LIBS) ../moo/libmoo.la


if MINGW_BUILD
medit_LDFLAGS += -mwindows
endif MINGW_BUILD

medit_SOURCES = medit.c


##############################################################################
##  mterm
##
mterm_LDFLAGS =
mterm_LDADD = $(MOO_LIBS) ../moo/libmoo.la
termbuffer_LDADD = $(mterm_LDADD)


if MINGW_BUILD
mterm_LDFLAGS += -mwindows
endif MINGW_BUILD

termbuffer_LDFLAGS = $(mterm_LDFLAGS)

mterm_SOURCES = mterm.c
termbuffer_SOURCES = termbuffer.c


##############################################################################
##  markup
##
markup_LDFLAGS =
markup_LDADD = $(MOO_LIBS) ../moo/libmoo.la


if MINGW_BUILD
markup_LDFLAGS += -mwindows
endif MINGW_BUILD

markup_SOURCES = markup.c


testpaned_LDFLAGS =
testpaned_LDADD = $(MOO_LIBS) ../moo/libmoo.la
if MINGW_BUILD
testpaned_LDFLAGS += -mwindows
endif MINGW_BUILD
testpaned_SOURCES = testpaned.c

testfileview_LDFLAGS =
testfileview_LDADD = $(MOO_LIBS) ../moo/libmoo.la
if MINGW_BUILD
testfileview_LDFLAGS += -mwindows
endif MINGW_BUILD
testfileview_SOURCES = testfileview.c

testpanedfileview_LDFLAGS =
testpanedfileview_LDADD = $(MOO_LIBS) ../moo/libmoo.la
if MINGW_BUILD
testpanedfileview_LDFLAGS += -mwindows
endif MINGW_BUILD
testpanedfileview_SOURCES = testpanedfileview.c
