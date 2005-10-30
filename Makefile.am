EXTRA_DIST = makewin mingw-configure

DIST_SUBDIRS = moo tests m4
SUBDIRS = moo

if !MOO_OS_CYGWIN
SUBDIRS += tests
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = moo.pc

ACLOCAL_AMFLAGS = -I m4
