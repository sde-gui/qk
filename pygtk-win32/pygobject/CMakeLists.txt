configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)
add_definitions(-DHAVE_CONFIG_H)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(glib STATIC
    ../${PYGOBJECT_SRC_DIR}/glib/pyglib.c
    ../${PYGOBJECT_SRC_DIR}/glib/pyglib.h
    ../${PYGOBJECT_SRC_DIR}/glib/pyglib-private.h
    ../${PYGOBJECT_SRC_DIR}/glib/pyglib-python-compat.h
)

add_definitions(-DPLATFORM_WIN32)
include_directories(
    ../${PYGOBJECT_SRC_DIR}/gi
    ../${PYGOBJECT_SRC_DIR}/gio
    ../${PYGOBJECT_SRC_DIR}/glib
    ../${PYGOBJECT_SRC_DIR}/gobject
)

#======================================================================================

add_library(_gobject MODULE
    CMakeLists.txt
    ../${PYGOBJECT_SRC_DIR}/gobject/gobjectmodule.c
    ../${PYGOBJECT_SRC_DIR}/gobject/pygboxed.c
    ../${PYGOBJECT_SRC_DIR}/gobject/pygboxed.h
    ../${PYGOBJECT_SRC_DIR}/gobject/pygenum.c
    ../${PYGOBJECT_SRC_DIR}/gobject/pygenum.h
    ../${PYGOBJECT_SRC_DIR}/gobject/pygflags.c
    ../${PYGOBJECT_SRC_DIR}/gobject/pygflags.h
    ../${PYGOBJECT_SRC_DIR}/gobject/pyginterface.c
    ../${PYGOBJECT_SRC_DIR}/gobject/pyginterface.h
    ../${PYGOBJECT_SRC_DIR}/gobject/pygobject.c
    ../${PYGOBJECT_SRC_DIR}/gobject/pygobject.h
    ../${PYGOBJECT_SRC_DIR}/gobject/pygobject-private.h
    ../${PYGOBJECT_SRC_DIR}/gobject/pygparamspec.c
    ../${PYGOBJECT_SRC_DIR}/gobject/pygparamspec.h
    ../${PYGOBJECT_SRC_DIR}/gobject/pygpointer.c
    ../${PYGOBJECT_SRC_DIR}/gobject/pygpointer.h
    ../${PYGOBJECT_SRC_DIR}/gobject/pygtype.c
    ../${PYGOBJECT_SRC_DIR}/gobject/pygtype.h
)

target_link_libraries(_gobject
    glib
    ${GTK_LIBRARIES}
    ${PYTHON_LIBRARIES}
)

SET_TARGET_PROPERTIES(_gobject PROPERTIES SUFFIX ".pyd")
SET_TARGET_PROPERTIES(_gobject PROPERTIES COMPILE_FLAGS -DPY_SSIZE_T_CLEAN)

set(GOBJECT_DEST_DIR ${MEDIT_PY_LIBDIR}/gobject)

install(TARGETS _gobject LIBRARY DESTINATION ${GOBJECT_DEST_DIR})
install(FILES
    ../${PYGOBJECT_SRC_DIR}/gobject/__init__.py
    ../${PYGOBJECT_SRC_DIR}/gobject/constants.py
    ../${PYGOBJECT_SRC_DIR}/gobject/propertyhelper.py
    DESTINATION ${GOBJECT_DEST_DIR})

#======================================================================================

add_library(_glib MODULE
    CMakeLists.txt
    ../${PYGOBJECT_SRC_DIR}/glib/glibmodule.c
    ../${PYGOBJECT_SRC_DIR}/glib/pygiochannel.c
    ../${PYGOBJECT_SRC_DIR}/glib/pygiochannel.h
    ../${PYGOBJECT_SRC_DIR}/glib/pygoptioncontext.c
    ../${PYGOBJECT_SRC_DIR}/glib/pygoptioncontext.h
    ../${PYGOBJECT_SRC_DIR}/glib/pygoptiongroup.c
    ../${PYGOBJECT_SRC_DIR}/glib/pygoptiongroup.h
    ../${PYGOBJECT_SRC_DIR}/glib/pygmaincontext.c
    ../${PYGOBJECT_SRC_DIR}/glib/pygmaincontext.h
    ../${PYGOBJECT_SRC_DIR}/glib/pygmainloop.c
    ../${PYGOBJECT_SRC_DIR}/glib/pygmainloop.h
    ../${PYGOBJECT_SRC_DIR}/glib/pygsource.c
    ../${PYGOBJECT_SRC_DIR}/glib/pygsource.h
    ../${PYGOBJECT_SRC_DIR}/glib/pygspawn.c
    ../${PYGOBJECT_SRC_DIR}/glib/pygspawn.h
)

target_link_libraries(_glib
    glib
    ${GTK_LIBRARIES}
    ${PYTHON_LIBRARIES}
)

SET_TARGET_PROPERTIES(_glib PROPERTIES SUFFIX ".pyd")
SET_TARGET_PROPERTIES(_gobject PROPERTIES COMPILE_FLAGS -DPY_SSIZE_T_CLEAN)

set(GLIB_DEST_DIR ${MEDIT_PY_LIBDIR}/glib)

install(TARGETS _glib LIBRARY DESTINATION ${GLIB_DEST_DIR})
install(FILES
    ../${PYGOBJECT_SRC_DIR}/glib/__init__.py
    ../${PYGOBJECT_SRC_DIR}/glib/option.py
    DESTINATION ${GLIB_DEST_DIR})

#======================================================================================

DEFS2C(../${PYGOBJECT_SRC_DIR}/gio gio gio.c gio.defs
    --override ${CMAKE_CURRENT_SOURCE_DIR}/../${PYGOBJECT_SRC_DIR}/gio/gio.override
    --register gio-types.defs)

add_library(_gio MODULE
    CMakeLists.txt
    ../${PYGOBJECT_SRC_DIR}/gio/giomodule.c
    ../${PYGOBJECT_SRC_DIR}/gio/pygio-utils.c
    ../${PYGOBJECT_SRC_DIR}/gio/pygio-utils.h
    gio.c
)

target_link_libraries(_gio
    glib
    ${GTK_LIBRARIES}
    ${PYTHON_LIBRARIES}
)

SET_TARGET_PROPERTIES(_gio PROPERTIES SUFFIX ".pyd")

set(GLIB_DEST_DIR ${MEDIT_PY_LIBDIR}/gtk-2.0/gio)

install(TARGETS _gio LIBRARY DESTINATION ${GLIB_DEST_DIR})
install(FILES
    ../${PYGOBJECT_SRC_DIR}/gio/__init__.py
    DESTINATION ${GLIB_DEST_DIR})

#======================================================================================
