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
    xdgmimealias.c      \
    xdgmimealias.h      \
    xdgmime.c           \
    xdgmimecache.c      \
    xdgmimecache.h      \
    xdgmimeglob.c       \
    xdgmimeglob.h       \
    xdgmime.h           \
    xdgmimeint.c        \
    xdgmimeint.h        \
    xdgmimemagic.c      \
    xdgmimemagic.h      \
    xdgmimeparent.c     \
    xdgmimeparent.h

noinst_PROGRAMS = test-mime

test_mime_LDADD = libxdgmime.la
test_mime_SOURCES = test-mime.c
