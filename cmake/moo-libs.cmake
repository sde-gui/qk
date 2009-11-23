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

MACRO(MOO_ADD_SOURCE_FILE_SET name subdir)
  _MOO_MODULE_SOURCES_VAR_NAMES(${name} _moo_s _moo_bs _moo_st)

  IF(_moo_code_module_created_${name})
    MESSAGE(FATAL_ERROR "MOO_ADD_SOURCE_FILE_SET(${name}) called twice")
  ENDIF(_moo_code_module_created_${name})
  SET(_moo_code_module_created_${name} 1)

  SET(_moo_source_file_prefix "")
  IF(NOT "" STREQUAL "${subdir}")
    SET(_moo_source_file_prefix "${subdir}/")
  ENDIF(NOT "" STREQUAL "${subdir}")

#   SET(${name}_cmake_dummy ${CMAKE_CURRENT_BINARY_DIR}/${name}-cmake-dummy.c)
#   ADD_CUSTOM_COMMAND(OUTPUT ${${name}_cmake_dummy}
#                      COMMAND ${CMAKE_COMMAND} -D LIBNAME=${name} -D OUTPUT=${${name}_cmake_dummy} -P ${CMAKE_SOURCE_DIR}/moo/gendummy.cmake
#                      DEPENDS ${${name}_stamps} ${CMAKE_SOURCE_DIR}/moo/gendummy.cmake)
#   SET_SOURCE_FILES_PROPERTIES(${${name}_cmake_dummy} PROPERTIES GENERATED 1)

  IF(_moo_source_file_prefix)
    SET(_moo_sources_set_tmp ${${_moo_s}})
    SET(${_moo_s})
    FOREACH(_moo_source_file ${_moo_sources_set_tmp})
      LIST(APPEND ${_moo_s} "${_moo_source_file_prefix}${_moo_source_file}")
    ENDFOREACH(_moo_source_file)
  ENDIF(_moo_source_file_prefix)

  LIST(APPEND moo_all_sources ${${_moo_s}} ${${_moo_bs}} ${${_moo_st}})

ENDMACRO(MOO_ADD_SOURCE_FILE_SET)

MACRO(MOO_ADD_MOO_CODE_MODULE name)

  SET(_moo_code_module_subdir ${name})

  SET(_moo_arg_next_subdir)
  FOREACH(_moo_arg ${ARGN})
    IF(_moo_arg_next_subdir)
      SET(_moo_code_module_subdir ${_moo_arg})
      SET(_moo_arg_next_subdir)
    ELSEIF("${_moo_arg}" STREQUAL "SUBDIR")
      SET(_moo_arg_next_subdir 1)
    ELSE(_moo_arg_next_subdir)
      MESSAGE(FATAL_ERROR " Invalid argument `${_moo_arg}' to macro MOO_ADD_CODE_MODULE")
    ENDIF(_moo_arg_next_subdir)
  ENDFOREACH(_moo_arg)

  MOO_ADD_SOURCE_FILE_SET(${name} ${_moo_code_module_subdir})

  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/moo/${_moo_code_module_subdir} ${CMAKE_CURRENT_BINARY_DIR}/${_moo_code_module_subdir})

ENDMACRO(MOO_ADD_MOO_CODE_MODULE)

MACRO(MOO_WIN32_RESOURCE rc_in module)
  IF(WIN32)
    SET(_moo_rc_file ${CMAKE_CURRENT_BINARY_DIR}/${module}.rc)
    SET_SOURCE_FILES_PROPERTIES(${_moo_rc_file} PROPERTIES GENERATED 1)
    SET(_moo_res_file ${CMAKE_CURRENT_BINARY_DIR}/${module}.res)
    CONFIGURE_FILE(${rc_in} ${_moo_rc_file} @ONLY)
    _MOO_MODULE_SOURCES_VAR_NAMES(${module} _moo_s _moo_bs _moo_st)
    IF(MSVC)
      LIST(APPEND ${_moo_s} ${_moo_rc_file})
    ELSE(MSVC)
      ADD_CUSTOM_COMMAND(OUTPUT ${_moo_res_file}
        COMMAND ${WINDRES_EXECUTABLE} -i ${_moo_rc_file} --input-format=rc -o ${_moo_res_file} -O coff
        DEPENDS ${_moo_rc_file}
      )
      SET_SOURCE_FILES_PROPERTIES(${_moo_res_file} PROPERTIES EXTERNAL_OBJECT 1 GENERATED 1)
      LIST(APPEND ${_moo_s} ${_moo_res_file})
      LIST(APPEND ${module}_libs ${_moo_res_file})
      # FIXME rebuilding just doesn't work
    ENDIF(MSVC)
  ENDIF(WIN32)
ENDMACRO(MOO_WIN32_RESOURCE)
