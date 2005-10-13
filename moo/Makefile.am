#
#   moo/Makefile.am
#

LTVERSION = 0:1:0

DIST_SUBDIRS =  \
    mooapp      \
    mooutils    \
    mooedit     \
    moopython   \
    mooterm

mooincludedir = $(includedir)/moo-1.0
mooinclude_HEADERS = moo.h
moolibincludedir = $(libdir)/moo-1.0/include
moolibinclude_HEADERS = moo-config.h

EXTRA_DIST = $(mooinclude_HEADERS) $(moolibinclude_HEADERS)


# lib_LTLIBRARIES = libmoo-1.0.la
noinst_LTLIBRARIES = libmoo-1.0.la

libmoo_1_0_la_SOURCES = \
    dummy.c

all-am: moo-config.h

moo_libadd =
subdirs =

if MOO_BUILD_UTILS
subdirs += mooutils
moo_libadd += mooutils/libmooutils.la
endif

if MOO_BUILD_EDIT
subdirs += mooedit
moo_libadd += mooedit/libmooedit.la
endif

if MOO_BUILD_TERM
subdirs += mooterm
if !MOO_OS_CYGWIN
moo_libadd += mooterm/libmooterm.la
endif !MOO_OS_CYGWIN
endif

if MOO_BUILD_APP
subdirs += mooapp
if !MOO_OS_CYGWIN
moo_libadd += mooapp/libmooapp.la
endif !MOO_OS_CYGWIN
endif

if MOO_USE_PYGTK
subdirs += moopython
moo_libadd += moopython/libmoopython.la
endif

libmoo_1_0_la_LIBADD = $(moo_libadd)
libmoo_1_0_la_LDFLAGS  = -version-info $(LTVERSION) -export-symbols-regex "^[^_].*" -no-undefined

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

moo_la_SOURCES = $(libmoo_1_0_SOURCES)

if MOO_OS_MINGW
mingw_flags = -no-undefined
endif MOO_OS_MINGW

moo_la_LDFLAGS = -module -avoid-version -export-symbols-regex initmoo $(mingw_flags)

moo_la_LIBADD =         \
    $(libmoo_la_LIBADD) \
    $(GTK_LIBS)         \
    $(XML_LIBS)         \
    $(PYTHON_LDFLAGS)   \
    $(PYTHON_EXTRA_LIBS)

endif BUILD_PYMOO
