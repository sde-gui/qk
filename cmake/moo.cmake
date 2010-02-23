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

SET(MOO_CMAKE_COMMAND "${CMAKE_COMMAND}"
  -D "MOO_SOURCE_DIR=${MOO_SOURCE_DIR}"
  -D "MOO_BINARY_DIR=${MOO_BINARY_DIR}"
  -D "GLIB_GENMARSHAL_EXECUTABLE=${GLIB_GENMARSHAL_EXECUTABLE}"
  -D "GDK_PIXBUF_CSOURCE_EXECUTABLE=${GDK_PIXBUF_CSOURCE_EXECUTABLE}"
  -D "PKG_CONFIG_EXECUTABLE=${PKG_CONFIG_EXECUTABLE}"
  -D "PYTHON_EXECUTABLE=${PYTHON_EXECUTABLE}"
)


# ##########################################################################
# #
# # Code modules, automake-style
# #
#
# MACRO(_MOO_MODULE_SOURCES_VAR_NAMES module out_sources_var out_builtsources_var out_stamps_var)
#   STRING(TOUPPER ${module} _moo_module_cap)
#   SET(${out_sources_var} ${_moo_module_cap}_SOURCES)
#   SET(${out_builtsources_var} ${_moo_module_cap}_BUILT_SOURCES)
#   SET(${out_stamps_var} ${_moo_module_cap}_STAMPS)
# ENDMACRO(_MOO_MODULE_SOURCES_VAR_NAMES)
#
# MACRO(_MOO_ALL_MODULE_SOURCES module outvar)
#   _MOO_MODULE_SOURCES_VAR_NAMES(${module} _moo_s _moo_bs _moo_st)
#   SET(${outvar} ${${_moo_s}} ${${_moo_bs}} ${${_moo_st}})
# ENDMACRO(_MOO_ALL_MODULE_SOURCES)
#
# MACRO(MOO_GENERATE_SOURCE_FILE module)
#   MOO_COLLECT_ARGS(ARG OUTPUT _moo_output
#                    ARG COMMAND _moo_command
#                    ARG WORKING_DIRECTORY _moo_working_directory
#                    ARG DEPENDS _moo_depends
#                    ARGN ${ARGN})
#
#   SET(_moo_output_tmp ${_moo_output})
#   FOREACH(_moo_f IN LISTS _moo_output_tmp)
#   ENDFOREACH(_moo_f)
#   GET_FILENAME_COMPONENT(VarName FileName
#                          PATH|ABSOLUTE|NAME|EXT|NAME_WE|REALPATH
#                          [CACHE])
#
# #   FOREACH(_moo_arg ${ARGN})
# #     IF("${_moo_arg}" STREQUAL "OUTPUT")
# #       SET(_moo_what OUTPUT)
# #     ELSEIF("${_moo_arg}" STREQUAL "COMMAND")
# #       SET(_moo_what COMMAND)
# #     ELSEIF("${_moo_arg}" STREQUAL "COMMAND")
# #
# #     IF(_moo_what EQUAL 0)
# #
# #     ENDIF(_moo_what EQUAL 0)
# #   ENDFOREACH(_moo_arg)
# #
# #   FOREACH(_moo_f ${stampfile} ${outputfiles})
# #     GET_FILENAME_COMPONENT(_moo_d ${_moo_f} ABSOLUTE)
# #     GET_FILENAME_COMPONENT(_moo_d ${_moo_d} PATH)
# #     FILE(MAKE_DIRECTORY ${_moo_d})
# #   ENDFOREACH(_moo_f)
# #   ADD_CUSTOM_COMMAND(OUTPUT ${stampfile} ${ARGN})
# #   _MOO_MODULE_SOURCES_VAR_NAMES(${module} _moo_s _moo_bs _moo_st)
# #   LIST(APPEND ${_moo_st} ${stampfile})
# #   SET_SOURCE_FILES_PROPERTIES(${stampfile} PROPERTIES EXTERNAL_OBJECT 1 GENERATED 1)
# #   LIST(APPEND ${_moo_bs} ${outputfiles})
# #   SET_SOURCE_FILES_PROPERTIES(${outputfiles} PROPERTIES GENERATED 1)
# #   MOO_ADD_CLEAN_FILES(${stampfile} ${outputfiles})
# ENDMACRO(MOO_GENERATE_SOURCE_FILE)

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

# MACRO(MOO_ADD_LIBRARY libname)
#   _MOO_ALL_MODULE_SOURCES(${libname} _moo_all_sources)
#   ADD_LIBRARY(${libname} ${ARGN} ${STAMPS})
# ENDMACRO(MOO_ADD_LIBRARY)

# MACRO(MOO_MAKE_SOURCE_FILES outvar)
#   SET(${outvar})
#   FOREACH(f ${ARGN})
#     GET_FILENAME_COMPONENT(f ${f} ABSOLUTE)
#     LIST(APPEND ${outvar} ${f})
#   ENDFOREACH(f)
# ENDMACRO(MOO_MAKE_SOURCE_FILES)

# MACRO(MOO_GET_ARGS outvar1 sep1 outvar2)
#   SET(${outvar1})
#   SET(${outvar2})
#   SET(__doing_part2)
#   FOREACH(arg ${ARGN})
#     IF(${__doing_part2})
#       LIST(APPEND ${outvar2} ${arg})
#     ELSEIF("${arg}" STREQUAL "${sep1}")
#       SET(__doing_part2 1)
#     ELSE(${__doing_part2})
#       LIST(APPEND ${outvar1} ${arg})
#     ENDIF(${__doing_part2})
#   ENDFOREACH(arg)
# ENDMACRO(MOO_GET_ARGS)

# MACRO(MOO_SUBDIR_NAME outvar)
#   STRING(LENGTH "${CMAKE_BINARY_DIR}" _moo_top_len)
#   STRING(LENGTH "${CMAKE_CURRENT_BINARY_DIR}" _moo_total_len)
#   IF(${_moo_top_len} EQUAL ${_moo_total_len})
#     SET(${outvar} ".")
#   ELSE(${_moo_top_len} EQUAL ${_moo_total_len})
#     MATH(EXPR _moo_sub_len "${_moo_total_len} - ${_moo_top_len} - 1")
#     MATH(EXPR _moo_top_len "${_moo_top_len} + 1")
#     STRING(SUBSTRING "${CMAKE_CURRENT_BINARY_DIR}" ${_moo_top_len} ${_moo_sub_len} ${outvar})
#   ENDIF(${_moo_top_len} EQUAL ${_moo_total_len})
# ENDMACRO(MOO_SUBDIR_NAME)

# -%- indent-width:2 -%-
