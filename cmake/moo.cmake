MACRO(MOO_DEFINE_VERSIONS name version)
  STRING(REGEX REPLACE "([^.]+)[.].*" "\\1" ${name}_MAJOR_VERSION ${version})
  STRING(REGEX REPLACE "[^.]+[.]([^.]+).*" "\\1" ${name}_MINOR_VERSION ${version})
  STRING(REGEX REPLACE "[^.]+[.][^.]+[.]([^.]+)" "\\1" ${name}_MICRO_VERSION ${version})
  SET(${name}_VERSION "\"${version}\"")
  SET(${name}_VERSION_UNQUOTED "${version}")
ENDMACRO(MOO_DEFINE_VERSIONS)

FIND_PACKAGE(PythonInterp)
IF(NOT PYTHONINTERP_FOUND)
  MESSAGE(FATAL_ERROR "Python not found")
ENDIF(NOT PYTHONINTERP_FOUND)

IF(WIN32 AND NOT MSVC)
  FIND_PROGRAM(WINDRES_EXECUTABLE windres
               DOC "Path to the windres executable")
  IF(NOT WINDRES_EXECUTABLE)
    MESSAGE(FATAL_ERROR "Could not find windres")
  ENDIF(NOT WINDRES_EXECUTABLE)
ENDIF(WIN32 AND NOT MSVC)

SET(MOO_CMAKE_COMMAND "${CMAKE_COMMAND}"
  -D "MOO_SOURCE_DIR=${MOO_SOURCE_DIR}"
  -D "MOO_BINARY_DIR=${MOO_BINARY_DIR}"
  -D "GLIB_GENMARSHAL_EXECUTABLE=${GLIB_GENMARSHAL_EXECUTABLE}"
  -D "GDK_PIXBUF_CSOURCE_EXECUTABLE=${GDK_PIXBUF_CSOURCE_EXECUTABLE}"
  -D "PKG_CONFIG_EXECUTABLE=${PKG_CONFIG_EXECUTABLE}"
  -D "PYTHON_EXECUTABLE=${PYTHON_EXECUTABLE}"
)

CONFIGURE_FILE(${MOO_SOURCE_DIR}/moo/glade2c.cmake.in ${CMAKE_BINARY_DIR}/moo/glade2c.cmake @ONLY)
MACRO(MOO_GEN_GXML)
  FOREACH(_mgg_gladefile ${ARGN})
    STRING(REGEX REPLACE "(.*)\\.glade" "\\1-gxml.h" _mgg_header ${_mgg_gladefile})
    SET(_mgg_header_full ${CMAKE_CURRENT_BINARY_DIR}/${_mgg_header})
    SET(_mgg_header_stamp ${_mgg_header_full}.stamp)
    MOO_ADD_GENERATED_FILE(
      ${_mgg_header_stamp} ${_mgg_header_full}
      COMMAND ${MOO_CMAKE_COMMAND} -D INPUT=${_mgg_gladefile} -D OUTPUT=${_mgg_header} -D SRCDIR=${CMAKE_CURRENT_SOURCE_DIR} -P "${CMAKE_BINARY_DIR}/moo/glade2c.cmake"
      DEPENDS ${_mgg_gladefile} ${CMAKE_BINARY_DIR}/moo/glade2c.cmake ${MOO_GLADE2C_PY}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
  ENDFOREACH(_mgg_gladefile)
ENDMACRO(MOO_GEN_GXML)

MACRO(MOO_GEN_ENUMS enummodule)
  SET(_moo_enum_input ${enummodule}-enums-in.py)
  SET(_moo_enum_output ${enummodule}-enums)
  GET_FILENAME_COMPONENT(_moo_enum_gen_py ${CMAKE_SOURCE_DIR}/moo/genenums.py ABSOLUTE)
  GET_FILENAME_COMPONENT(_moo_enum_stamp ${CMAKE_CURRENT_BINARY_DIR}/${_moo_enum_output}.stamp ABSOLUTE)
  MOO_ADD_GENERATED_FILE(
    ${_moo_enum_stamp}
    "${CMAKE_CURRENT_SOURCE_DIR}/${_moo_enum_output}.c;${CMAKE_CURRENT_SOURCE_DIR}/${_moo_enum_output}.h"
    COMMAND ${PYTHON_EXECUTABLE} ${_moo_enum_gen_py} ${enummodule} ${_moo_enum_input} ${_moo_enum_output} ${_moo_enum_stamp}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_moo_enum_input} ${_moo_enum_gen_py}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
ENDMACRO(MOO_GEN_ENUMS)

MACRO(MOO_GEN_UIXML)
  FOREACH(uifile ${ARGN})
    GET_FILENAME_COMPONENT(basename ${uifile} NAME_WE)
    STRING(REPLACE "-" "_" varname ${basename})
    SET(varname ${varname}_ui_xml)
    SET(header ${basename}-ui.h)
    SET(header_full ${CMAKE_CURRENT_BINARY_DIR}/${header})
    SET(header_stamp ${header_full}.stamp)
    MOO_ADD_GENERATED_FILE(
      ${header_stamp} ${header_full}
      COMMAND ${PYTHON_EXECUTABLE} ${MOO_XML2H_PY}
                                   ${CMAKE_CURRENT_SOURCE_DIR}/${uifile}
                                   ${CMAKE_CURRENT_BINARY_DIR}/${header}
                                   ${varname}
      DEPENDS ${uifile} ${MOO_XML2H_PY}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
  ENDFOREACH(uifile)
ENDMACRO(MOO_GEN_UIXML)

# -%- indent-width:2 -%-
