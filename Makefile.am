EXTRA_DIST = makewin mingw-configure

DIST_SUBDIRS = moo tests
SUBDIRS = moo

if !MOO_OS_CYGWIN
SUBDIRS += tests
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = moo.pc
