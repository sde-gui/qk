configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)
add_definitions(-DHAVE_CONFIG_H)
include_directories(SYSTEM ${CMAKE_CURRENT_BINARY_DIR})

add_library(_cairo MODULE
    CMakeLists.txt
    ../${PYCAIRO_SRC_DIR}/src/cairomodule.c
    ../${PYCAIRO_SRC_DIR}/src/context.c
    ../${PYCAIRO_SRC_DIR}/src/font.c
    ../${PYCAIRO_SRC_DIR}/src/path.c
    ../${PYCAIRO_SRC_DIR}/src/pattern.c
    ../${PYCAIRO_SRC_DIR}/src/matrix.c
    ../${PYCAIRO_SRC_DIR}/src/private.h
    ../${PYCAIRO_SRC_DIR}/src/pycairo.h
    ../${PYCAIRO_SRC_DIR}/src/surface.c
)

target_link_libraries(_cairo
    ${MOO_GTK_DIR}/lib/cairo.lib
    ${PYTHON_LIBRARIES}
)

SET_TARGET_PROPERTIES(_cairo PROPERTIES SUFFIX ".pyd")
SET_TARGET_PROPERTIES(_cairo PROPERTIES LINK_FLAGS /EXPORT:init_cairo)

install(TARGETS _cairo LIBRARY DESTINATION ${MEDIT_PY_LIBDIR}/cairo)
install(FILES
    ../${PYCAIRO_SRC_DIR}/src/__init__.py
    DESTINATION ${MEDIT_PY_LIBDIR}/cairo)
