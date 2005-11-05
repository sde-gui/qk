EXTRA_DIST = makewin mingw-configure

DIST_SUBDIRS = tests m4
SUBDIRS = .

if !MOO_OS_CYGWIN
SUBDIRS += tests
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = moo.pc

ACLOCAL_AMFLAGS = -I m4


DEFAULT_INCLUDES =
AM_CFLAGS =                                 \
    -I$(top_builddir)                       \
    -I$(top_srcdir)/$(MOO_SRC_PREFIX)   \
    -I$(top_builddir)/$(MOO_SRC_PREFIX) \
    $(MOO_CFLAGS)                           \
    $(PYTHON_INCLUDES)                      \
    $(PYGTK_CFLAGS)                         \
    $(MOO_DEBUG_CFLAGS)                     \
    $(CFLAGS)

if !MOO_OS_CYGWIN
noinst_LTLIBRARIES = libmoo-1.0.la
endif

###########################################################################

EXTRA_PROGRAMS =
moo_prefix = moo
XML2H = $(srcdir)/moo/mooutils/xml2h.sh
include moo/Makefile.incl
include moo/mooutils/Makefile.incl
include moo/mooedit/Makefile.incl
include moo/mooterm/Makefile.incl
include moo/mooapp/Makefile.incl
include moo/moopython/Makefile.incl

###########################################################################

CLEANFILES = $(moo_cleanfiles)

if !MOO_OS_CYGWIN
libmoo_1_0_la_LDFLAGS  = -version-info $(LTVERSION) -export-symbols-regex "^[^_].*" -no-undefined
libmoo_1_0_la_SOURCES = $(moo_built_sources) $(moo_sources)
BUILT_SOURCES = $(moo_built_sources)
nodist_libmoo_1_0_la_SOURCES = $(moo_nodist_sources)
if MOO_OS_MINGW
libmoo_1_0_la_SOURCES += $(moo_win32_sources)
endif
if MOO_OS_UNIX
libmoo_1_0_la_SOURCES += $(moo_unix_sources)
endif
endif

EXTRA_DIST +=            \
$(moo_sources)           \
$(moo_win32_sources)     \
$(moo_unix_sources)      \
$(moo_built_sources)     \
$(moo_extra_dist)
