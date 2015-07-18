add_definitions(-DLUA_USE_APICHECK)

if(MOO_OS_DARWIN)
add_definitions(-DLUA_USE_MACOSX)
elseif(MOO_OS_WIN32)
else(MOO_OS_DARWIN)
add_definitions(-DLUA_USE_POSIX -DLUA_USE_DLOPEN)
LIST(APPEND moo_libadd -ldl)
endif(MOO_OS_DARWIN)

LIST(APPEND moolua_sources
    moolua/lua/lfs.h
    moolua/lua/lfs.cpp
    moolua/lua/moolua.h
    moolua/lua/moolua.cpp
    moolua/lua/luaall.cpp
)

LIST(APPEND moolua_extra_dist
    moolua/lua/COPYRIGHT
    moolua/lua/lapi.c
    moolua/lua/lapi.h
    moolua/lua/lauxlib.c
    moolua/lua/lauxlib.h
    moolua/lua/lbaselib.c
    moolua/lua/lcode.c
    moolua/lua/lcode.h
    moolua/lua/ldblib.c
    moolua/lua/ldebug.c
    moolua/lua/ldebug.h
    moolua/lua/ldo.c
    moolua/lua/ldo.h
    moolua/lua/ldump.c
    moolua/lua/lfunc.c
    moolua/lua/lfunc.h
    moolua/lua/lgc.c
    moolua/lua/lgc.h
    moolua/lua/linit.c
    moolua/lua/liolib.c
    moolua/lua/llex.c
    moolua/lua/llex.h
    moolua/lua/llimits.h
    moolua/lua/lmathlib.c
    moolua/lua/lmem.c
    moolua/lua/lmem.h
    moolua/lua/loadlib.c
    moolua/lua/lobject.c
    moolua/lua/lobject.h
    moolua/lua/lopcodes.c
    moolua/lua/lopcodes.h
    moolua/lua/loslib.c
    moolua/lua/lparser.c
    moolua/lua/lparser.h
    moolua/lua/lstate.c
    moolua/lua/lstate.h
    moolua/lua/lstring.c
    moolua/lua/lstring.h
    moolua/lua/ltable.c
    moolua/lua/ltable.h
    moolua/lua/ltablib.c
    moolua/lua/ltm.c
    moolua/lua/ltm.h
    moolua/lua/lua.h
    moolua/lua/luaall.cpp
    moolua/lua/luaconf.h
    moolua/lua/lualib.h
    moolua/lua/lundump.c
    moolua/lua/lundump.h
    moolua/lua/lvm.c
    moolua/lua/lvm.h
    moolua/lua/lzio.c
    moolua/lua/lzio.h
    moolua/lua/README
    moolua/lua/slnudata.c
    moolua/lua/slnunico.c
)

# zzz
# luadir = $(MOO_DATA_DIR)/lua
# EXTRA_DIST += moolua/lua/_moo
# install-data-local: install-lua-moo
# uninstall-local: uninstall-lua-moo
# install-lua-moo:
# 	$(MKDIR_P) $(DESTDIR)$(luadir)/_moo
# 	cd $(srcdir) && $(INSTALL_DATA) moolua/lua/_moo/*.lua $(DESTDIR)$(luadir)/_moo/
# uninstall-lua-moo:
# 	rm -f $(DESTDIR)$(luadir)/_moo/*.lua
