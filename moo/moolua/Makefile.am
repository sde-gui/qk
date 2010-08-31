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
	moo-tests-lua.h	\
	luaall.cpp

EXTRA_DIST =		\
	COPYRIGHT	\
	lapi.c		\
	lapi.h		\
	lauxlib.c	\
	lauxlib.h	\
	lbaselib.c	\
	lcode.c		\
	lcode.h		\
	ldblib.c	\
	ldebug.c	\
	ldebug.h	\
	ldo.c		\
	ldo.h		\
	ldump.c		\
	lfunc.c		\
	lfunc.h		\
	lgc.c		\
	lgc.h		\
	linit.c		\
	liolib.c	\
	llex.c		\
	llex.h		\
	llimits.h	\
	lmathlib.c	\
	lmem.c		\
	lmem.h		\
	loadlib.c	\
	lobject.c	\
	lobject.h	\
	lopcodes.c	\
	lopcodes.h	\
	loslib.c	\
	lparser.c	\
	lparser.h	\
	lstate.c	\
	lstate.h	\
	lstring.c	\
	lstring.h	\
	ltable.c	\
	ltable.h	\
	ltablib.c	\
	ltm.c		\
	ltm.h		\
	lua.h		\
	luaall.cpp	\
	luaconf.h	\
	lualib.h	\
	lundump.c	\
	lundump.h	\
	lvm.c		\
	lvm.h		\
	lzio.c		\
	lzio.h		\
	README		\
	slnudata.c	\
	slnunico.c
