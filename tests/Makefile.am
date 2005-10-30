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
noinst_PROGRAMS += mterm #termbuffer
endif
if MOO_BUILD_UTILS
# noinst_PROGRAMS += markup miniglade
endif

bin_PROGRAMS += $(medit)


AM_CFLAGS =                     \
    -I$(top_builddir)           \
    $(MOO_CFLAGS)               \
    $(MOO_DEBUG_CFLAGS)         \
    $(CFLAGS)                   \
    $(GTK_CFLAGS)

ldflags = -export-dynamic
ldadd =                     \
    ../moo/libmoo-1.0.la    \
    $(MOO_LIBS)             \
    $(PYTHON_LDFLAGS)       \
    $(PYTHON_EXTRA_LIBS)

if MOO_OS_MINGW
ldflags += -mwindows
endif MOO_OS_MINGW


##############################################################################
##  medit
##
medit_LDFLAGS = $(ldflags)
medit_LDADD = $(ldadd)

medit_SOURCES = medit-ui.h

if MOO_BUILD_APP
medit_SOURCES += medit-app.c
else
medit_SOURCES += medit.c
endif


##############################################################################
##  langparser
##
langparser_LDFLAGS = $(ldflags)
langparser_LDADD = $(ldadd)

langparser_SOURCES = testparser.c


##############################################################################
##  mterm
##
mterm_LDFLAGS = $(ldflags)
mterm_LDADD = $(ldadd)
termbuffer_LDFLAGS = $(ldflags)
termbuffer_LDADD = $(ldadd)

mterm_SOURCES = mterm.c
termbuffer_SOURCES = termbuffer.c


##############################################################################
##  markup
##
markup_LDFLAGS = $(ldflags)
markup_LDADD = $(ldadd)

markup_SOURCES = markup.c


##############################################################################
##  miniglade
##
miniglade_LDFLAGS = $(ldflags)
miniglade_LDADD = $(ldadd)

miniglade_SOURCES = miniglade.c


##############################################################################

testpaned_LDFLAGS = $(ldflags)
testpaned_LDADD = $(ldadd)
testpaned_SOURCES = testpaned.c

testfileview_LDFLAGS = $(ldflags)
testfileview_LDADD = $(ldadd)
testfileview_SOURCES = testfileview.c

testpanedfileview_LDFLAGS = $(ldflags)
testpanedfileview_LDADD = $(ldadd)
testpanedfileview_SOURCES = testpanedfileview.c
