glib_2_8_sources =				\
	glib-2.8/glib/gmappedfile-real.h	\
	glib-2.8/glib/gmappedfile.h		\
	glib-2.8/glib/gmappedfile.c		\
	glib-2.8/glib/galias.h

glib_2_12_sources =				\
	glib-2.12/glib/gbase64-real.h		\
	glib-2.12/glib/gbase64.h		\
	glib-2.12/glib/gbase64.c

glib_2_14_sources =				\
	glib-2.14/glib/gregex-real.h		\
	glib-2.14/glib/gregex.h			\
	glib-2.14/glib/gregex.c

EXTRA_DIST =					\
	$(glib_2_8_sources)			\
	$(glib_2_12_sources)			\
	$(glib_2_14_sources)

noinst_LTLIBRARIES = libnewgtk.la

libnewgtk_la_SOURCES =

if !GLIB_2_14
libnewgtk_la_SOURCES +=	$(glib_2_14_sources)
if MOO_BUILD_PCRE
pcre_cflags = -I$(srcdir)/..
else
pcre_cflags = -DUSE_SYSTEM_PCRE
endif
endif

if !GLIB_2_12
libnewgtk_la_SOURCES +=	$(glib_2_12_sources)
endif

if !GLIB_2_8
libnewgtk_la_SOURCES +=	$(glib_2_8_sources)
endif

AM_CFLAGS =				\
	$(pcre_cflags)			\
	$(MOO_CFLAGS)			\
	$(MOO_W_NO_UNUSED)		\
	$(MOO_W_NO_WRITE_STRINGS)	\
	$(MOO_W_NO_SIGN_COMPARE)
