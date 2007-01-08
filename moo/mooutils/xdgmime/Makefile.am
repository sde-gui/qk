mooxdgmime_sources =	\
	xdgmimealias.c	\
	xdgmimealias.h	\
	xdgmime.c	\
	xdgmimecache.c	\
	xdgmimecache.h	\
	xdgmimeglob.c	\
	xdgmimeglob.h	\
	xdgmime.h	\
	xdgmimeint.c	\
	xdgmimeint.h	\
	xdgmimemagic.c	\
	xdgmimemagic.h	\
	xdgmimeparent.c	\
	xdgmimeparent.h

noinst_LTLIBRARIES = libxdgmime.la
libxdgmime_la_SOURCES = $(mooxdgmime_sources)
AM_CFLAGS =			\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)	\
	-Wno-sign-compare	\
	-Wno-unused
