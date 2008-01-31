noinst_LTLIBRARIES = liblua.la libmoolua.la

# a hack, needed for 'make whatever.lo' work
AM_CFLAGS =			\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)	\
	$(MOO_LUA_CFLAGS)	\
	-DLUA_USE_APICHECK

libmoolua_la_LIBADD = liblua.la
if MOO_ENABLE_UNIT_TESTS
AM_CFLAGS += -DSRCDIR=\"$(shell cd $(srcdir) && pwd)\" -DTOP_SRCDIR=\"$(shell cd $(top_srcdir) && pwd)\"
endif
libmoolua_la_SOURCES =		\
	ext/lfs.h		\
	ext/lfs.c		\
	moolua.h		\
	moolua.c		\
	slnunico.c

noinst_PROGRAMS = lua
lua_SOURCES = lua.c
lua_LDADD = libmoolua.la $(MOO_LIBS) $(MOO_OBJC_LIBS) -lreadline
lua_CFLAGS = $(AM_CFLAGS) -DLUA_USE_READLINE

liblua_la_LDFLAGS =
liblua_la_SOURCES =		\
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
	test/testustring.lua	\
	test/testunicode.lua	\
	test/munit.lua		\
	slnudata.c
