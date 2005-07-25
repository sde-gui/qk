#
#   moo/Makefile.am
#

DIST_SUBDIRS =  \
    mooapp      \
    mooutils    \
    mooedit     \
    mooterm     \
    mooui

subdirs =

noinst_LTLIBRARIES = libmoo.la
libmoo_la_LIBADD =
libmoo_la_SOURCES = dummy.c
moo_pygtk_flags =

if BUILD_MOOUTILS
subdirs += mooutils
libmoo_la_LIBADD += mooutils/libmooutils.la
moo_pygtk_flags += -DBUILD_MOOUTILS
endif

if BUILD_MOOUI
subdirs += mooui
moo_pygtk_flags += -DBUILD_MOOUI
libmoo_la_LIBADD += mooui/libmooui.la
endif

if BUILD_MOOEDIT
subdirs += mooedit
moo_pygtk_flags += -DBUILD_MOOEDIT
libmoo_la_LIBADD += mooedit/libmooedit.la
endif

if BUILD_MOOTERM
subdirs += mooterm
moo_pygtk_flags += -DBUILD_MOOTERM
if !CYGWIN_BUILD
libmoo_la_LIBADD += mooterm/libmooterm.la
endif !CYGWIN_BUILD
endif

if BUILD_MOOAPP
subdirs += mooapp
moo_pygtk_flags += -DBUILD_MOOAPP
if !CYGWIN_BUILD
libmoo_la_LIBADD += mooapp/libmooapp.la
endif !CYGWIN_BUILD
endif

SUBDIRS = $(subdirs) .

DEFAULT_INCLUDES =
AM_CFLAGS =                                 \
    -I$(top_builddir)                       \
    $(MOO_CFLAGS)                           \
    $(PYTHON_INCLUDES)                      \
    $(PYGTK_CFLAGS)                         \
    $(DEBUG_CFLAGS)                         \
    $(CFLAGS)                               \
    -DMOO_VERSION=\"$(MOO_VERSION)\"        \
    -DMOO_VERSION_MAJOR=$(MOO_VERSION_MAJOR)\
    -DMOO_VERSION_MINOR=$(MOO_VERSION_MINOR)\
    -DMOO_VERSION_MICRO=$(MOO_VERSION_MICRO)\
    $(moo_pygtk_flags)

EXTRA_DIST = moo-pygtk.c

if USE_PYGTK
if !CYGWIN_BUILD
libmoo_la_SOURCES += moo-pygtk.c
endif !CYGWIN_BUILD
endif USE_PYGTK


##############################################################################
# moo module
#
if BUILD_PYMOO

pyexec_LTLIBRARIES = moo.la

moo_la_SOURCES = moo-pygtk.c

if MINGW_BUILD
mingw_flags = -no-undefined
endif MINGW_BUILD

moo_la_LDFLAGS = -module -avoid-version -export-symbols-regex initmoo $(mingw_flags)

moo_la_LIBADD =         \
    $(libmoo_la_LIBADD) \
    $(GTK_LIBS)         \
    $(XML_LIBS)         \
    $(PYTHON_LDFLAGS)   \
    $(PYTHON_EXTRA_LIBS)

endif BUILD_PYMOO
