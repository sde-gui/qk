noinst_LTLIBRARIES = libgregex.la
libgregex_la_SOURCES =	\
	gregex-real.h	\
	gregex.h	\
	gregex.c

if MOO_BUILD_PCRE
pcre_cflags = -I$(srcdir)/../..
else
pcre_cflags = -DUSE_SYSTEM_PCRE
endif

AM_CFLAGS =			\
	-I$(top_builddir)	\
	$(pcre_cflags)		\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)	\
	$(MOO_W_NO_UNUSED)

EXTRA_DIST =			\
	galias.h		\
	galiasdef.c
