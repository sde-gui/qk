#
#   tests/Makefile.am
#

EXTRA_PROGRAMS = medit mterm markup termbuffer testfileview testpaned testpanedfileview miniglade langparser
bin_PROGRAMS =
noinst_PROGRAMS =

EXTRA_DIST =        \
    pyapp.py.in     \
    medit-ui.xml

BUILT_SOURCES = medit-ui.h
medit-ui.h: medit-ui.xml
	sh $(srcdir)/../moo/mooutils/xml2h.sh MEDIT_UI $(srcdir)/medit-ui.xml > medit-ui.h

all-am: pyapp.py medit-ui.h

if MOO_BUILD_APP
medit = medit
endif
if MOO_BUILD_EDIT
medit = medit
endif
if MOO_BUILD_TERM
# noinst_PROGRAMS += mterm termbuffer
endif
if MOO_BUILD_UTILS
# noinst_PROGRAMS += markup miniglade
endif

bin_PROGRAMS += $(medit)


AM_CFLAGS =                     \
    -I$(top_builddir)           \
    $(MOO_CFLAGS)               \
    $(DEBUG_CFLAGS)             \
    $(CFLAGS)                   \
    $(GTK_CFLAGS)


##############################################################################
##  medit
##
medit_LDFLAGS = -export-dynamic
medit_LDADD =               \
    ../moo/libmoo-1.0.la    \
    $(MOO_LIBS)             \
    $(PYTHON_LDFLAGS)       \
    $(PYTHON_EXTRA_LIBS)


if MOO_OS_MINGW
medit_LDFLAGS += -mwindows
endif MOO_OS_MINGW

medit_SOURCES = medit-ui.h

if MOO_BUILD_APP
medit_SOURCES += medit-app.c
else
medit_SOURCES += medit.c
endif


##############################################################################
##  langparser
##
langparser_LDFLAGS =
langparser_LDADD = $(MOO_LIBS) ../moo/libmoo-1.0.la


if MOO_OS_MINGW
langparser_LDFLAGS += -mwindows
endif MOO_OS_MINGW

langparser_SOURCES = testparser.c


##############################################################################
##  mterm
##
mterm_LDFLAGS =
mterm_LDADD = $(MOO_LIBS) ../moo/libmoo-1.0.la
termbuffer_LDADD = $(mterm_LDADD)


if MOO_OS_MINGW
mterm_LDFLAGS += -mwindows
endif MOO_OS_MINGW

termbuffer_LDFLAGS = $(mterm_LDFLAGS)

mterm_SOURCES = mterm.c
termbuffer_SOURCES = termbuffer.c


##############################################################################
##  markup
##
markup_LDFLAGS =
markup_LDADD = $(MOO_LIBS) ../moo/libmoo-1.0.la


if MOO_OS_MINGW
markup_LDFLAGS += -mwindows
endif MOO_OS_MINGW

markup_SOURCES = markup.c


##############################################################################
##  miniglade
##
miniglade_LDFLAGS =
miniglade_LDADD = $(MOO_LIBS) ../moo/libmoo-1.0.la
miniglade_CFLAGS = $(AM_CFLAGS)

miniglade_SOURCES = miniglade.c


##############################################################################

testpaned_LDFLAGS =
testpaned_LDADD = $(MOO_LIBS) ../moo/libmoo-1.0.la
if MOO_OS_MINGW
testpaned_LDFLAGS += -mwindows
endif MOO_OS_MINGW
testpaned_SOURCES = testpaned.c

testfileview_LDFLAGS =
testfileview_LDADD = $(MOO_LIBS) ../moo/libmoo-1.0.la
if MOO_OS_MINGW
testfileview_LDFLAGS += -mwindows
endif MOO_OS_MINGW
testfileview_SOURCES = testfileview.c

testpanedfileview_LDFLAGS =
testpanedfileview_LDADD = $(MOO_LIBS) ../moo/libmoo-1.0.la
if MOO_OS_MINGW
testpanedfileview_LDFLAGS += -mwindows
endif MOO_OS_MINGW
testpanedfileview_SOURCES = testpanedfileview.c
