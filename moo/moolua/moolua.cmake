SET(moolua_sources
    moolua/moolua.cmake
    moolua/medit-lua.h
    moolua/medit-lua.cpp
    moolua/mooluaplugin.cpp
    moolua/moolua-tests.h
    moolua/moolua-tests.cpp
    moolua/moo-lua-api-util.h
    moolua/moo-lua-api-util.cpp
    moolua/moo-lua-api.h
    moolua/gtk-lua-api.h
)

SET(moolua_extra_files
    moolua/lua-default-init.lua
    moolua/lua-module-init.lua
    moolua/lua-plugin-init.lua
)

SET(genlua_files
    ${CMAKE_SOURCE_DIR}/api/genlua.py
    ${CMAKE_SOURCE_DIR}/api/mpi/__init__.py
    ${CMAKE_SOURCE_DIR}/api/mpi/module.py
    ${CMAKE_SOURCE_DIR}/api/mpi/luawriter.py
)

include(moolua/lua/lua.cmake)

XML2H(moolua/lua-default-init.lua moolua/lua-default-init.h LUA_DEFAULT_INIT)
XML2H(moolua/lua-module-init.lua moolua/lua-module-init.h LUA_MODULE_INIT)
XML2H(moolua/lua-plugin-init.lua moolua/lua-plugin-init.h LUA_PLUGIN_INIT)

add_custom_command(OUTPUT moolua/moo-lua-api.cpp
    COMMAND ${MOO_PYTHON} ${CMAKE_SOURCE_DIR}/api/genlua.py
                --include-header moolua/moo-lua-api.h
		--import ${CMAKE_SOURCE_DIR}/api/gtk.xml
                --output moolua/moo-lua-api.cpp
		${CMAKE_SOURCE_DIR}/api/moo.xml
    MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/api/moo.xml
    DEPENDS ${genlua_files} ${CMAKE_SOURCE_DIR}/api/gtk.xml)
list(APPEND built_moolua_sources moolua/moo-lua-api.cpp)

add_custom_command(OUTPUT moolua/gtk-lua-api.cpp
    COMMAND ${MOO_PYTHON} ${CMAKE_SOURCE_DIR}/api/genlua.py
                --include-header moolua/gtk-lua-api.h
                --output moolua/gtk-lua-api.cpp
		${CMAKE_SOURCE_DIR}/api/gtk.xml
    MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/api/gtk.xml
    DEPENDS ${genlua_files})
list(APPEND built_moolua_sources moolua/gtk-lua-api.cpp)
