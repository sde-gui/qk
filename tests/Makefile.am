#
#   tests/Makefile.am
#

EXTRA_PROGRAMS = medit mterm markup editor
noinst_PROGRAMS =

EXTRA_DIST = pyapp.py.in
all-am: pyapp.py

if BUILD_MOOAPP
noinst_PROGRAMS += editor
endif
if BUILD_MOOEDIT
noinst_PROGRAMS += medit
endif
if BUILD_MOOTERM
noinst_PROGRAMS += mterm
endif
if BUILD_MOOUTILS
noinst_PROGRAMS += markup
endif


DEFAULT_INCLUDES =
AM_CFLAGS =                     \
    -I$(top_builddir)           \
    $(MOO_CFLAGS)               \
    $(M_CFLAGS)                 \
    $(CFLAGS)                   \
    $(GTK_CFLAGS)
AM_CXXFLAGS =                   \
    -I$(top_builddir)           \
    $(MOO_CFLAGS)               \
    $(M_CXXFlAGS)               \
    $(CXXFLAGS)                 \
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

editor_SOURCES = editor.cpp


##############################################################################
##  medit
##
medit_LDFLAGS =
medit_LDADD = $(MOO_LIBS) ../moo/libmoo.la


if MINGW_BUILD
medit_LDFLAGS += -mwindows
endif MINGW_BUILD

medit_SOURCES = medit.cpp


##############################################################################
##  mterm
##
mterm_LDFLAGS =
mterm_LDADD = $(MOO_LIBS) ../moo/libmoo.la


if MINGW_BUILD
mterm_LDFLAGS += -mwindows
endif MINGW_BUILD

mterm_SOURCES = \
    mterm.cpp


##############################################################################
##  markup
##
markup_LDFLAGS =
markup_LDADD = $(MOO_LIBS) ../moo/libmoo.la


if MINGW_BUILD
markup_LDFLAGS += -mwindows
endif MINGW_BUILD

markup_SOURCES = \
    markup.cpp

