set(medit_app_sources
    medit-app/medit-app.cmake
    medit-app/mem-debug.h
    medit-app/run-tests.h
    medit-app/parse.h
    medit-app/main.c
)

set(built_medit_app_sources)

if(MOO_OS_WIN32)
    set(top_srcdir ${CMAKE_SOURCE_DIR})
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/medit-app/medit.rc.in ${CMAKE_CURRENT_BINARY_DIR}/medit-app/medit.rc)
    list(APPEND built_medit_app_sources ${CMAKE_CURRENT_BINARY_DIR}/medit-app/medit.rc)
    list(APPEND medit_app_sources ${CMAKE_CURRENT_SOURCE_DIR}/medit-app/medit.rc.in)
endif()
