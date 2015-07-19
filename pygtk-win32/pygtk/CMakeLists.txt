SET(pygtk_major_version 2)
SET(pygtk_minor_version 24)
SET(pygtk_micro_version 0)
SET(pygtk_version ${pygtk_major_version}.${pygtk_minor_version}.${pygtk_micro_version})

configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)
add_definitions(-DHAVE_CONFIG_H)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(PYGTK_SRC_DIR "../pygtk-${pygtk_version}")
set(PYGTK_DEST_DIR ${MEDIT_PY_LIBDIR}/gtk-2.0)

include_directories(
    ${PYGTK_SRC_DIR}
    ${PYGTK_SRC_DIR}/gtk
    ../${PYGOBJECT_SRC_DIR}/gi
    ../${PYGOBJECT_SRC_DIR}/gio
    ../${PYGOBJECT_SRC_DIR}/glib
    ../${PYGOBJECT_SRC_DIR}/gobject
    ../${PYCAIRO_SRC_DIR}/src
)

#======================================================================================

add_library(atk MODULE
    CMakeLists.txt
    atk.c
    ${PYGTK_SRC_DIR}/atk.override
    ${PYGTK_SRC_DIR}/atkmodule.c
)

target_link_libraries(atk
    ${GTK_LIBRARIES}
    ${PYTHON_LIBRARIES}
)

SET_TARGET_PROPERTIES(atk PROPERTIES SUFFIX ".pyd")

install(TARGETS atk LIBRARY DESTINATION ${PYGTK_DEST_DIR})

DEFS2C(${PYGTK_SRC_DIR} atk atk.c atk.defs
    --register pango-types.defs
    --register atk-types.defs
    --register gtk/gtk-base-types.defs
    --override atk.override)

#======================================================================================

add_library(pango MODULE
    CMakeLists.txt
    pango.c
    ${PYGTK_SRC_DIR}/pango.override
    ${PYGTK_SRC_DIR}/pangomodule.c
)

target_link_libraries(pango
    ${GTK_LIBRARIES}
    ${PYTHON_LIBRARIES}
)

SET_TARGET_PROPERTIES(pango PROPERTIES SUFFIX ".pyd")

install(TARGETS pango LIBRARY DESTINATION ${PYGTK_DEST_DIR})

DEFS2C(${PYGTK_SRC_DIR} pango pango.c pango.defs
    --register pango-types.defs
    --register atk-types.defs
    --register gtk/gtk-base-types.defs
    --override pango.override)

#======================================================================================

add_library(pangocairo MODULE
    CMakeLists.txt
    pangocairo.c
    ${PYGTK_SRC_DIR}/pangocairo.override
    ${PYGTK_SRC_DIR}/pangocairomodule.c
)

target_link_libraries(pangocairo
    ${GTK_LIBRARIES}
    ${PYTHON_LIBRARIES}
)

SET_TARGET_PROPERTIES(pangocairo PROPERTIES SUFFIX ".pyd")

install(TARGETS pangocairo LIBRARY DESTINATION ${PYGTK_DEST_DIR})

DEFS2C(${PYGTK_SRC_DIR} pangocairo pangocairo.c pangocairo.defs
    --register pango-types.defs
    --register atk-types.defs
    --register gtk/gtk-base-types.defs
    --override pangocairo.override)

#======================================================================================

set(GTK_DEFS
    gtk-base.defs
    gtk-2.10.defs 
    gtk-2.12.defs 
    gtk-2.14.defs 
    gtk-2.16.defs 
    gtk-2.18.defs 
    gtk-2.20.defs 
    gtk-2.22.defs 
    gtk-2.24.defs 
)

set(GDK_DEFS
    gdk-base.defs
    gdk-2.10.defs
    gdk-2.12.defs 
    gdk-2.14.defs 
    gdk-2.16.defs 
    gdk-2.18.defs 
    gdk-2.20.defs 
    gdk-2.22.defs 
    gdk-2.24.defs 
)

set(GTK_TYPES_DEFS
    gtk-base-types.defs
    gtk-2.10-types.defs
    gtk-2.12-types.defs
    gtk-2.14-types.defs
    gtk-2.16-types.defs
    gtk-2.18-types.defs
    gtk-2.20-types.defs
    gtk-2.22-types.defs
    gtk-2.24-types.defs
)

set(GDK_TYPES_DEFS
    gdk-base-types.defs
)

macro(MAKEDEFS)
    set(_out ${ARGV0})
    set(_main_file ${ARGV1})
    set(_additional_args ${ARGV})
    list(REMOVE_AT _additional_args 0 1)

    add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${PYGTK_SRC_DIR}/gtk/${_out}
        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/pygtk-win32/pygobject-${pygobject_version}/codegen/createdefs.py
            ${_out} ${_additional_args}
        MAIN_DEPENDENCY ${PYGTK_SRC_DIR}/gtk/${_main_file}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${PYGTK_SRC_DIR}/gtk)
endmacro(MAKEDEFS)

MAKEDEFS(gtk.defs gtk-base.defs ${GTK_DEFS})
MAKEDEFS(gdk.defs gdk-base.defs ${GDK_DEFS})
MAKEDEFS(gtk-types.defs gtk-base-types.defs ${GTK_TYPES_DEFS})
MAKEDEFS(gdk-types.defs gdk-base-types.defs ${GDK_TYPES_DEFS})

set(DEFS2C_ADDITIONAL_DEFS_FILES gtk-types.defs)
DEFS2C(${PYGTK_SRC_DIR}/gtk gtk gtk.c gtk.defs
    --register ../../${PYGOBJECT_SRC_DIR}/gio/gio-types.defs
    --register ../pango-types.defs
    --register ../atk-types.defs
    --register gdk-types.defs
    --register gtk-types.defs
    --override gtk.override
)

set(DEFS2C_ADDITIONAL_DEFS_FILES gdk-types.defs)
DEFS2C(${PYGTK_SRC_DIR}/gtk gdk gdk.c gdk.defs
    --register ../../${PYGOBJECT_SRC_DIR}/gio/gio-types.defs
    --register ../pango-types.defs
    --register ../atk-types.defs
    --register gdk-types.defs
    --register gtk-types.defs
    --override gdk.override
)

add_library(_gtk MODULE
    CMakeLists.txt
    gtk.c
    gdk.c
    ${PYGTK_SRC_DIR}/gtk/gtk-types.c
    ${PYGTK_SRC_DIR}/gtk/gtkmodule.c
    ${PYGTK_SRC_DIR}/gtk/gtkobject-support.c
    ${PYGTK_SRC_DIR}/gtk/pygtk.h
    ${PYGTK_SRC_DIR}/gtk/pygtk-private.h
    ${PYGTK_SRC_DIR}/gtk/pygtkcellrenderer.h
    ${PYGTK_SRC_DIR}/gtk/pygtkcellrenderer.c
    ${PYGTK_SRC_DIR}/gtk/pygtktreemodel.h
    ${PYGTK_SRC_DIR}/gtk/pygtktreemodel.c
    ${PYGTK_SRC_DIR}/gtk/gtk.defs
    ${PYGTK_SRC_DIR}/gtk/gdk.defs
    ${PYGTK_SRC_DIR}/gtk/gtk-types.defs
    ${PYGTK_SRC_DIR}/gtk/gdk-types.defs
)

target_link_libraries(_gtk
    ${GTK_LIBRARIES}
    ${PYTHON_LIBRARIES}
)

SET_TARGET_PROPERTIES(_gtk PROPERTIES SUFFIX ".pyd")

install(TARGETS _gtk LIBRARY DESTINATION ${PYGTK_DEST_DIR}/gtk)
install(FILES
    ${PYGTK_SRC_DIR}/gtk/__init__.py
    ${PYGTK_SRC_DIR}/gtk/_lazyutils.py
    ${PYGTK_SRC_DIR}/gtk/compat.py
    ${PYGTK_SRC_DIR}/gtk/deprecation.py
    ${PYGTK_SRC_DIR}/gtk/keysyms.py
    DESTINATION ${PYGTK_DEST_DIR}/gtk)

install(FILES
    pygtk.pth
    DESTINATION ${MEDIT_PY_LIBDIR}
)
