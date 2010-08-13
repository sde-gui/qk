MACRO(MOO_ADD_COMPILE_FLAGS target flags)
  GET_TARGET_PROPERTY(__moo_prop_value ${target} COMPILE_FLAGS)
  IF(__moo_prop_value)
    SET_TARGET_PROPERTIES(${target} PROPERTIES COMPILE_FLAGS "${flags}")
  ELSE(__moo_prop_value)
    SET_TARGET_PROPERTIES(${target} PROPERTIES COMPILE_FLAGS "${__moo_prop_value} ${flags}")
  ENDIF(__moo_prop_value)
ENDMACRO(MOO_ADD_COMPILE_FLAGS)

MACRO(MOO_ADD_DEFINITIONS target defs)
  GET_TARGET_PROPERTY(__moo_prop_value ${target} COMPILE_DEFINITIONS)
  IF(__moo_prop_value)
    LIST(APPEND __moo_prop_value ${defs})
  ELSE(__moo_prop_value)
    SET(__moo_prop_value ${defs})
  ENDIF(__moo_prop_value)
  SET_TARGET_PROPERTIES(${target} PROPERTIES COMPILE_DEFINITIONS "${__moo_prop_value}")
ENDMACRO(MOO_ADD_DEFINITIONS)

# MACRO(MOO_ADD_SOURCE_FILE_SET name subdir)
#   _MOO_MODULE_SOURCES_VAR_NAMES(${name} _moo_s _moo_bs _moo_st)
#
#   IF(_moo_code_module_created_${name})
#     MESSAGE(FATAL_ERROR "MOO_ADD_SOURCE_FILE_SET(${name}) called twice")
#   ENDIF(_moo_code_module_created_${name})
#   SET(_moo_code_module_created_${name} 1)
#
#   SET(_moo_source_file_prefix "")
#   IF(NOT "" STREQUAL "${subdir}")
#     SET(_moo_source_file_prefix "${subdir}/")
#   ENDIF(NOT "" STREQUAL "${subdir}")
#
# #   SET(${name}_cmake_dummy ${CMAKE_CURRENT_BINARY_DIR}/${name}-cmake-dummy.c)
# #   ADD_CUSTOM_COMMAND(OUTPUT ${${name}_cmake_dummy}
# #                      COMMAND ${CMAKE_COMMAND} -D LIBNAME=${name} -D OUTPUT=${${name}_cmake_dummy} -P ${CMAKE_SOURCE_DIR}/moo/gendummy.cmake
# #                      DEPENDS ${${name}_stamps} ${CMAKE_SOURCE_DIR}/moo/gendummy.cmake)
# #   SET_SOURCE_FILES_PROPERTIES(${${name}_cmake_dummy} PROPERTIES GENERATED 1)
#
#   IF(_moo_source_file_prefix)
#     SET(_moo_sources_set_tmp ${${_moo_s}})
#     SET(${_moo_s})
#     FOREACH(_moo_source_file ${_moo_sources_set_tmp})
#       LIST(APPEND ${_moo_s} "${_moo_source_file_prefix}${_moo_source_file}")
#     ENDFOREACH(_moo_source_file)
#   ENDIF(_moo_source_file_prefix)
#
#   LIST(APPEND moo_all_sources ${${_moo_s}} ${${_moo_bs}} ${${_moo_st}})
#
# ENDMACRO(MOO_ADD_SOURCE_FILE_SET)

MACRO(MOO_ADD_LIBRARY libname)
  STRING(TOUPPER ${libname} _mal_mod)
  GET_DIRECTORY_PROPERTY(_mal_stamps MOO_STAMPS)
  GET_DIRECTORY_PROPERTY(_mal_built_sources MOO_BUILT_SOURCES)
#   FOREACH(_moo_source_file ${${_mal_mod}_SOURCES} ${_mal_built_sources})
#     IF("${_moo_source_file}" MATCHES ".*[.]c$")
#       IF(NOT "${_moo_source_file}" STREQUAL "moofontsel.c")
#         set_source_files_properties(${_moo_source_file} PROPERTIES LANGUAGE CXX)
#       ENDIF(NOT "${_moo_source_file}" STREQUAL "moofontsel.c")
#     ENDIF("${_moo_source_file}" MATCHES ".*[.]c$")
#   ENDFOREACH(_moo_source_file)
  ADD_LIBRARY(${libname} STATIC ${ARGN} ${${_mal_mod}_SOURCES} ${_mal_stamps} ${_mal_built_sources})
ENDMACRO(MOO_ADD_LIBRARY)

MACRO(MOO_ADD_MOO_CODE_MODULE)
  MOO_ADD_LIBRARY(${ARGN})
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
ENDMACRO(MOO_ADD_MOO_CODE_MODULE)

MACRO(MOO_WIN32_RESOURCE rc_in module module_sources module_libs)
  IF(WIN32)
    SET(_moo_rc_file ${CMAKE_CURRENT_BINARY_DIR}/${module}.rc)
    SET_SOURCE_FILES_PROPERTIES(${_moo_rc_file} PROPERTIES GENERATED 1)
    SET(_moo_res_file ${CMAKE_CURRENT_BINARY_DIR}/${module}.res)
    CONFIGURE_FILE(${rc_in} ${_moo_rc_file} @ONLY)
    IF(MSVC)
      LIST(APPEND ${module_sources} ${_moo_rc_file})
    ELSE(MSVC)
      ADD_CUSTOM_COMMAND(OUTPUT ${_moo_res_file}
        COMMAND ${WINDRES_EXECUTABLE} -i ${_moo_rc_file} --input-format=rc -o ${_moo_res_file} -O coff
        DEPENDS ${_moo_rc_file}
      )
      SET_SOURCE_FILES_PROPERTIES(${_moo_res_file} PROPERTIES EXTERNAL_OBJECT 1 GENERATED 1)
      LIST(APPEND ${module_sources} ${_moo_res_file})
      LIST(APPEND ${module_libs} ${_moo_res_file})
      # FIXME rebuilding just doesn't work
    ENDIF(MSVC)
  ENDIF(WIN32)
ENDMACRO(MOO_WIN32_RESOURCE)
