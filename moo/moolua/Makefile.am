noinst_LTLIBRARIES = libustring.la libmoolua.la

# a hack, needed for 'make whatever.lo' work
AM_CFLAGS =			\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)	\
	-DLUA_USE_APICHECK	\
	$(MOO_LUA_CFLAGS)	\
	-DSRCDIR=\"$(shell cd $(srcdir) && pwd)\"

libustring_la_CFLAGS =		\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)	\
	-DSRCDIR=\"$(shell cd $(srcdir) && pwd)\"
libustring_la_SOURCES =		\
	ustring.h		\
	ustring.c

libmoolua_la_LIBADD = libustring.la
libmoolua_la_LDFLAGS =
libmoolua_la_CFLAGS =		\
	-DLUA_USE_APICHECK	\
	$(MOO_LUA_CFLAGS)
libmoolua_la_SOURCES =		\
	ext/lfs.h		\
	ext/lfs.c		\
	moolua.h		\
	lapi.c			\
	lcode.c			\
	ldebug.c		\
	ldo.c			\
	ldump.c			\
	lfunc.c			\
	lgc.c			\
	llex.c			\
	lmem.c			\
	lobject.c		\
	lopcodes.c		\
	lparser.c		\
	lstate.c		\
	lstring.c		\
	ltable.c		\
	ltm.c			\
	lundump.c		\
	lvm.c			\
	lzio.c			\
	lauxlib.c		\
	lbaselib.c		\
	ldblib.c		\
	liolib.c		\
	lmathlib.c		\
	loslib.c		\
	ltablib.c		\
	lstrlib.c		\
	loadlib.c		\
	linit.c			\
	lapi.h			\
	lauxlib.h		\
	lcode.h			\
	ldebug.h		\
	ldo.h			\
	lfunc.h			\
	lgc.h			\
	llex.h			\
	llimits.h		\
	lmem.h			\
	lobject.h		\
	lopcodes.h		\
	lparser.h		\
	lstate.h		\
	lstring.h		\
	ltable.h		\
	ltm.h			\
	luaconf.h		\
	lua.h			\
	lualib.h		\
	lundump.h		\
	lvm.h			\
	lzio.h

EXTRA_DIST =			\
	COPYRIGHT		\
	ext/README.lfs		\
	test/testustring.lua
