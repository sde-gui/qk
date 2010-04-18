ADD_SUBDIRECTORY(plugins)

SET(MOOMOD_SOURCES
  moo-mod.c
  moopython-api.h
  moopython-utils.h
  moopython-utils.c
  moopython-loader.h
  moopython-loader.c
)

SET(PYMOO_SOURCES
  moopython-mod.c
  moopython-pygtkmod.h
  moopython-api.h
  moopython-loader.h
  moopython-loader.c
  moopython-utils.h
  moopython-utils.c
)

SET(MOOPYTHON_SOURCES
  moopython-builtin.h
  moopython-builtin.c
  moopython-api.h
  moopython-loader.h
  moopython-loader.c
  moopython-utils.h
  moopython-utils.c
)

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}/pygtk
  ${CMAKE_CURRENT_BINARY_DIR}/pygtk
  ${PYTHON_INCLUDE_DIRS}
  ${PYGTK_INCLUDE_DIRS}
)

ADD_DEFINITIONS(${PYTHON_DEFINITIONS} ${PYGTK_DEFINITIONS})

SET(MOOPYGTK_SOURCES
  pygtk/moo-pygtk.c
  pygtk/moo-pygtk.h
  pygtk/mooapp-mod.h
  pygtk/mooedit-mod.h
  pygtk/mooutils-mod.h
  pygtk/mooapp-mod.c
  pygtk/mooedit-mod.c
  pygtk/mooutils-mod.c
  pygtk/moo-mod.h
)

SET(mooedit_defs_files
  pygtk/mooeditor.defs
  pygtk/mooplugin.defs
  pygtk/moocommand.defs
)

SET(mooutils_defs_files
  pygtk/moofileview.defs
  pygtk/moopaned.defs
)

SET(mooutils_override_files
  pygtk/moopaned.override
)

IF(WIN32)
  SET(codegen_platform --platform win32)
ELSE(WIN32)
  SET(codegen_platform)
ENDIF(WIN32)

SET(codegen_files
  codegen/codegen.py
  codegen/argtypes.py
  codegen/argtypes_m.py
  codegen/reversewrapper.py
)

GET_FILENAME_COMPONENT(_moo_abs_srcdir ${CMAKE_CURRENT_SOURCE_DIR} ABSOLUTE)

SET(codegen_script ${_moo_abs_srcdir}/codegen/codegen.py)
SET(codegen ${PYTHON_EXECUTABLE} ${codegen_script} ${codegen_platform})

SET(MOO_UTILS_CODEGEN_ARGS
)

SET(MOO_EDIT_CODEGEN_ARGS
  --register ${_moo_abs_srcdir}/pygtk/mooutils-pygtk.defs
)

SET(MOO_APP_CODEGEN_ARGS
  --register ${_moo_abs_srcdir}/pygtk/mooutils-pygtk.defs
  --register ${_moo_abs_srcdir}/pygtk/mooedit-pygtk.defs
)

SET(MOO_PY2H ${_moo_abs_srcdir}/py2h.py)

MACRO(MOO_GEN_MOD_H _moo_comp)
  STRING(TOUPPER "${_moo_comp}" _moo_COMP)
  MOO_ADD_GENERATED_FILE(
    ${CMAKE_CURRENT_BINARY_DIR}/pygtk/moo${_moo_comp}-mod.h.stamp
    ${CMAKE_CURRENT_BINARY_DIR}/pygtk/moo${_moo_comp}-mod.h
    COMMAND ${CMAKE_COMMAND} -E make_directory pygtk
    COMMAND ${PYTHON_EXECUTABLE} ${MOO_PY2H}
            ${_moo_abs_srcdir}/pygtk/moo${_moo_comp}-mod.py
            ${CMAKE_CURRENT_BINARY_DIR}/pygtk/moo${_moo_comp}-mod.h
            MOO${_moo_COMP}_PY
    DEPENDS pygtk/moo${_moo_comp}-mod.py ${MOO_PY2H})
ENDMACRO(MOO_GEN_MOD_H)

MOO_GEN_MOD_H("")
FOREACH(_moo_comp utils edit app)
  STRING(TOUPPER ${_moo_comp} _moo_COMP)
  MOO_GEN_MOD_H(${_moo_comp})
  MOO_ADD_GENERATED_FILE(
    ${CMAKE_CURRENT_BINARY_DIR}/pygtk/moo${_moo_comp}-pygtk.stamp
    ${CMAKE_CURRENT_BINARY_DIR}/pygtk/moo${_moo_comp}-pygtk.c
    COMMAND ${CMAKE_COMMAND} -E make_directory pygtk
    COMMAND ${codegen}
      --prefix _moo_${_moo_comp}
      --load-types ${_moo_abs_srcdir}/codegen/argtypes_m.py
      --register ${PYGTK_DEFS_DIR}/gtk-types.defs
      --register ${PYGTK_DEFS_DIR}/gdk-types.defs
      ${MOO_${_moo_COMP}_CODEGEN_ARGS}
      --override ${_moo_abs_srcdir}/pygtk/moo${_moo_comp}-pygtk.override
      --outfile pygtk/moo${_moo_comp}-pygtk.c.tmp
      --outfilename pygtk/moo${_moo_comp}-pygtk.c
      ${_moo_abs_srcdir}/pygtk/moo${_moo_comp}-pygtk.defs
    COMMAND ${CMAKE_COMMAND} -E copy_if_different pygtk/moo${_moo_comp}-pygtk.c.tmp pygtk/moo${_moo_comp}-pygtk.c
    COMMAND ${CMAKE_COMMAND} -E remove pygtk/moo${_moo_comp}-pygtk.c.tmp
    COMMAND ${CMAKE_COMMAND} -E touch pygtk/moo${_moo_comp}-pygtk.stamp
    DEPENDS pygtk/moo${_moo_comp}-pygtk.defs
            pygtk/moo${_moo_comp}-pygtk.override
            ${moo${_moo_comp}_override_files}
            ${moo${_moo_comp}_defs_files}
            ${codegen_files})
ENDFOREACH(_moo_comp)

# noinst_LTLIBRARIES += libmoopygtk.la
#
# libmoopygtk_la_SOURCES = $(moopygtk_sources)
# nodist_libmoopygtk_la_SOURCES = $(nodist_moopygtk_sources)
# libmoopygtk_la_LIBADD =
#
# libmoopygtk_la_CFLAGS =
# 	-Ipygtk
# 	$(MOO_CFLAGS)
# 	$(MOO_W_NO_WRITE_STRINGS)
# 	$(MOO_W_NO_UNUSED)
# 	$(PYTHON_INCLUDES)
# 	$(PYGTK_CFLAGS)
#
# libmoopygtk_la_CXXFLAGS =
# 	-Ipygtk
# 	$(MOO_CXXFLAGS)
# 	$(MOO_W_NO_WRITE_STRINGS)
# 	$(MOO_W_NO_UNUSED)
# 	$(PYTHON_INCLUDES)
# 	$(PYGTK_CFLAGS)

INCLUDE_DIRECTORIES(${MOO_SOURCE_DIR})
MOO_ADD_LIBRARY(moopython ${MOOPYTHON_SOURCES} ${MOOPYGTK_SOURCES})

# -%- strip:true, indent-width:2 -%-
