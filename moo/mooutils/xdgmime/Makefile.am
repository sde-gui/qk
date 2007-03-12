mooxdgmime_sources =	\
	xdgmimealias.c	\
	xdgmimealias.h	\
	xdgmime.c	\
	xdgmimecache.c	\
	xdgmimecache.h	\
	xdgmimeglob.c	\
	xdgmimeglob.h	\
	xdgmime.h	\
	xdgmimeint.h	\
	xdgmimemagic.c	\
	xdgmimemagic.h	\
	xdgmimeparent.c	\
	xdgmimeparent.h

EXTRA_DIST = xdgmimeint.c

noinst_LTLIBRARIES = libxdgmime.la
libxdgmime_la_SOURCES = $(mooxdgmime_sources)
AM_CFLAGS =			\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)	\
	$(MOO_W_NO_SIGN_COMPARE)\
	$(MOO_W_NO_UNUSED)
