AM_CFLAGS =					\
	-I$(top_builddir)			\
	$(MOO_DEBUG_CFLAGS)			\
	$(MOO_CFLAGS)				\
	$(MOO_DEBUG_CFLAGS)			\
	$(MOO_W_NO_MISSING_FIELD_INITIALIZERS)	\
	$(MOO_W_NO_UNUSED)

noinst_LTLIBRARIES =				\
	libsmclient.la

libsmclient_la_SOURCES =			\
	eggsmclient.c				\
	eggsmclient.h				\
	eggsmclient-private.h

libsmclient_la_LIBADD =

if GDK_X11
AM_CFLAGS += -DEGG_SM_CLIENT_BACKEND_XSMP
libsmclient_la_LIBADD += -lSM -lICE
libsmclient_la_SOURCES += eggsmclient-xsmp.c
endif GDK_X11

# if GDK_WIN32
# AM_CFLAGS += -DEGG_SM_CLIENT_BACKEND_WIN32
# libsmclient_la_SOURCES += eggsmclient-win32.c
# endif

EXTRA_DIST =					\
	eggsmclient-xsmp.c			\
	eggsmclient-win32.c
