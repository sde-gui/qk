#
#   mooedit/xdgmime/Makefile.am
#

DEFAULT_INCLUDES =
AM_CFLAGS =                     \
    -I$(top_builddir)           \
    -Wall                       \
    -DXDG_PREFIX=_moo_edit_xdg  \
    -DG_LOG_DOMAIN=\"Xdg\"

noinst_LTLIBRARIES = libxdgmime.la

libxdgmime_la_SOURCES = \
    xdgmime.c       \
    xdgmime.h       \
    xdgmimealias.c  \
    xdgmimealias.h  \
    xdgmimeglob.c   \
    xdgmimeglob.h   \
    xdgmimeint.c    \
    xdgmimeint.h    \
    xdgmimemagic.c  \
    xdgmimemagic.h  \
    xdgmimeparent.c \
    xdgmimeparent.h

EXTRA_DIST = winfuncs.h winfuncs.c

noinst_PROGRAMS = test-mime

test_mime_LDADD = libxdgmime.la
test_mime_SOURCES = test-mime.c

if MINGW_BUILD
libxdgmime_la_SOURCES += winfuncs.h winfuncs.c
# libxdgmime_LDFLAGS = $(GLIB_LIBS)
AM_CFLAGS += $(GLIB_CFLAGS)
test_mime_LDADD += $(GLIB_LIBS)
endif MINGW_BUILD

