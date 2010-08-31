noinst_LTLIBRARIES = libmoolua.la
libmoolua_la_LIBADD =

AM_CPPFLAGS = $(MOO_CPPFLAGS) -DLUA_USE_APICHECK
AM_CFLAGS = $(MOO_CFLAGS)
AM_CXXFLAGS = $(MOO_CXXFLAGS)

if MOO_OS_DARWIN
AM_CPPFLAGS += -DLUA_USE_MACOSX
else !MOO_OS_DARWIN
if MOO_OS_WIN32
else !MOO_OS_WIN32
AM_CPPFLAGS += -DLUA_USE_POSIX -DLUA_USE_DLOPEN
libmoolua_la_LIBADD += -ldl
endif !MOO_OS_WIN32
endif !MOO_OS_DARWIN

libmoolua_la_SOURCES =	\
	ext/lfs.h	\
	ext/lfs.cpp	\
	moolua.h	\
	moolua.cpp	\
	moolua-tests.h	\
	luaall.cpp
