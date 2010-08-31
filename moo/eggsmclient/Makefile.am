noinst_LTLIBRARIES = libeggsmclient.la

AM_CPPFLAGS = $(MOO_CPPFLAGS)
AM_CFLAGS = $(MOO_CFLAGS)
AM_CXXFLAGS = $(MOO_CXXFLAGS)

libeggsmclient_la_LIBADD =

libeggsmclient_la_SOURCES =	\
	eggsmclient.c		\
	eggsmclient.h		\
	eggsmclient-mangle.h	\
	eggsmclient-private.h

if MOO_OS_WIN32
  libeggsmclient_la_SOURCES += eggsmclient-win32.c
else !MOO_OS_WIN32
if MOO_OS_DARWIN
  libeggsmclient_la_SOURCES += eggsmclient-dummy.c
else !MOO_OS_DARWIN
  AM_CFLAGS += -DEGG_SM_CLIENT_BACKEND_XSMP
  libeggsmclient_la_SOURCES += eggsmclient-xsmp.c eggdesktopfile.h eggdesktopfile.c
  libeggsmclient_la_LIBADD += -lICE -lSM
endif !MOO_OS_DARWIN
endif !MOO_OS_WIN32

# -%- strip:true -%-
