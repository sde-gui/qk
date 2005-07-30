#
#   moo/Makefile.am
#

DIST_SUBDIRS =  \
    mooapp      \
    mooutils    \
    mooedit     \
    moopython   \
    mooterm     \
    mooui

subdirs =

noinst_LTLIBRARIES = libmoo.la
libmoo_la_LIBADD =
libmoo_la_SOURCES = dummy.c

if BUILD_MOOUTILS
subdirs += mooutils
libmoo_la_LIBADD += mooutils/libmooutils.la
endif

if BUILD_MOOUI
subdirs += mooui
libmoo_la_LIBADD += mooui/libmooui.la
endif

if BUILD_MOOEDIT
subdirs += mooedit
libmoo_la_LIBADD += mooedit/libmooedit.la
endif

if BUILD_MOOTERM
subdirs += mooterm
if !CYGWIN_BUILD
libmoo_la_LIBADD += mooterm/libmooterm.la
endif !CYGWIN_BUILD
endif

if BUILD_MOOAPP
subdirs += mooapp
if !CYGWIN_BUILD
libmoo_la_LIBADD += mooapp/libmooapp.la
endif !CYGWIN_BUILD
endif

if USE_PYGTK
subdirs += moopython
libmoo_la_LIBADD += moopython/libmoopython.la
endif


SUBDIRS = $(subdirs) .

DEFAULT_INCLUDES =
AM_CFLAGS =                                 \
    -I$(top_builddir)                       \
    $(MOO_CFLAGS)                           \
    $(PYTHON_INCLUDES)                      \
    $(PYGTK_CFLAGS)                         \
    $(DEBUG_CFLAGS)                         \
    $(CFLAGS)


##############################################################################
# moo module
#
if BUILD_PYMOO

pyexec_LTLIBRARIES = moo.la

moo_la_SOURCES = $(libmoo_la_SOURCES)

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
