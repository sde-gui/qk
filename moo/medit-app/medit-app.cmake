set(medit_app_sources
    medit-app/mem-debug.h
    medit-app/run-tests.h
    medit-app/parse.h
    medit-app/main.c
    medit-app/medit-app.cmake
)

set(built_medit_app_sources)

if(MOO_OS_WIN32)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/medit-app/medit.rc.in ${CMAKE_CURRENT_BINARY_DIR}/medit-app/medit.rc)
    list(APPEND built_medit_app_sources ${CMAKE_CURRENT_BINARY_DIR}/medit-app/medit.rc)
endif()
