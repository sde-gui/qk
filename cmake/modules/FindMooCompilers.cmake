MACRO(_MOO_CHECK_COMPILER_LANG lang var)
  STRING(TOUPPER ${lang} _moo__ccl_lang)
  IF(NOT ${_moo__ccl_lang} STREQUAL C AND NOT ${_moo__ccl_lang} STREQUAL CXX)
    MOO_ERROR("Invalid lang '${lang}'")
  ENDIF(NOT ${_moo__ccl_lang} STREQUAL C AND NOT ${_moo__ccl_lang} STREQUAL CXX)
  SET(${var} ${_moo__ccl_lang})
ENDMACRO(_MOO_CHECK_COMPILER_LANG)

MACRO(_MOO_ADD_COMPILER_FLAGS lang buildtype)
  _MOO_CHECK_COMPILER_LANG(${lang} _moo__acf_lang)

  IF(${buildtype} STREQUAL DEBUG)
    SET(_moo__acf_build_sfxs _DEBUG)
  ELSEIF(${buildtype} STREQUAL RELEASE)
    SET(_moo__acf_build_sfxs _RELEASE _RELWITHDEBINFO _MINSIZEREL)
  ELSEIF(${buildtype} STREQUAL RELEASEONLY)
    SET(_moo__acf_build_sfxs _RELEASE)
  ELSEIF(${buildtype} STREQUAL RELWITHDEBINFO OR ${buildtype} STREQUAL MINSIZEREL)
    SET(_moo__acf_build_sfxs _${buildtype})
  ELSEIF(${buildtype} STREQUAL ALL)
    SET(_moo__acf_build_sfxs "")
  ELSE(${buildtype} STREQUAL DEBUG)
    MOO_ERROR("Invalid argument '${buildtype}'")
  ENDIF(${buildtype} STREQUAL DEBUG)

  FOREACH(_moo__acf_sfx ${_moo__acf_build_sfxs})
    FOREACH(_moo__acf_arg ${ARGN})
      SET(_moo__acf_var CMAKE_${_moo__acf_lang}_FLAGS${_moo__acf_sfx})
      SET(${_moo__acf_var} "${${_moo__acf_var}} ${_moo__acf_arg}")
    ENDFOREACH(_moo__acf_arg)
  ENDFOREACH(_moo__acf_sfx)
ENDMACRO(_MOO_ADD_COMPILER_FLAGS)

MACRO(MOO_ADD_C_FLAGS_DEBUG)
  _MOO_ADD_COMPILER_FLAGS(C DEBUG ${ARGN})
ENDMACRO(MOO_ADD_C_FLAGS_DEBUG)

MACRO(MOO_ADD_CXX_FLAGS_DEBUG)
  _MOO_ADD_COMPILER_FLAGS(CXX DEBUG ${ARGN})
ENDMACRO(MOO_ADD_CXX_FLAGS_DEBUG)

MACRO(MOO_ADD_C_FLAGS_RELEASE)
  _MOO_ADD_COMPILER_FLAGS(C RELEASE ${ARGN})
ENDMACRO(MOO_ADD_C_FLAGS_RELEASE)

MACRO(MOO_ADD_CXX_FLAGS_RELEASE)
  _MOO_ADD_COMPILER_FLAGS(CXX RELEASE ${ARGN})
ENDMACRO(MOO_ADD_CXX_FLAGS_RELEASE)

MACRO(MOO_ADD_C_FLAGS)
  _MOO_ADD_COMPILER_FLAGS(C ALL ${ARGN})
ENDMACRO(MOO_ADD_C_FLAGS)

MACRO(MOO_ADD_CXX_FLAGS)
  _MOO_ADD_COMPILER_FLAGS(CXX ALL ${ARGN})
ENDMACRO(MOO_ADD_CXX_FLAGS)

MACRO(__MOO_ADD_COMPILE_DEFINITIONS buildtype)
  SET(_moo_acd_do_add OFF)
  IF(MOO_DEBUG)
    SET(_moo_acd_debug ON)
    SET(_moo_acd_not_debug OFF)
  ELSE(MOO_DEBUG)
    SET(_moo_acd_debug OFF)
    SET(_moo_acd_not_debug ON)
  ENDIF(MOO_DEBUG)

  IF(${buildtype} STREQUAL DEBUG)
    SET(_moo_acd_do_add ${_moo_acd_debug})
  ELSEIF(${buildtype} STREQUAL RELEASE)
    SET(_moo_acd_do_add ${_moo_acd_not_debug})
  ELSEIF(${buildtype} STREQUAL RELEASEONLY)
    IF(MOO_BUILD_SUBTYPE STREQUAL RELEASE)
      SET(_moo_acd_do_add ON)
    ENDIF(MOO_BUILD_SUBTYPE STREQUAL RELEASE)
  ELSEIF(${buildtype} STREQUAL RELWITHDEBINFO OR ${buildtype} STREQUAL MINSIZEREL)
    IF(MOO_BUILD_SUBTYPE STREQUAL ${buildtype})
      SET(_moo_acd_do_add ON)
    ENDIF(MOO_BUILD_SUBTYPE STREQUAL ${buildtype})
  ELSEIF(${buildtype} STREQUAL ALL)
    SET(_moo_acd_do_add ON)
  ELSE(${buildtype} STREQUAL DEBUG)
    MOO_ERROR("Invalid argument '${buildtype}'")
  ENDIF(${buildtype} STREQUAL DEBUG)

  IF(_moo_acd_do_add)
    ADD_DEFINITIONS(${ARGN})
  ENDIF(_moo_acd_do_add)
ENDMACRO(__MOO_ADD_COMPILE_DEFINITIONS)

MACRO(MOO_ADD_COMPILE_DEFINITIONS)
  __MOO_ADD_COMPILE_DEFINITIONS(ALL ${ARGN})
ENDMACRO(MOO_ADD_COMPILE_DEFINITIONS)

MACRO(MOO_ADD_COMPILE_DEFINITIONS_DEBUG)
  __MOO_ADD_COMPILE_DEFINITIONS(DEBUG ${ARGN})
ENDMACRO(MOO_ADD_COMPILE_DEFINITIONS_DEBUG)

MACRO(MOO_ADD_COMPILE_DEFINITIONS_RELEASE)
  __MOO_ADD_COMPILE_DEFINITIONS(RELEASE ${ARGN})
ENDMACRO(MOO_ADD_COMPILE_DEFINITIONS_RELEASE)

MOO_ADD_COMPILE_DEFINITIONS_DEBUG(-DMOO_DEBUG -DDEBUG -D_DEBUG)


###########################################################################
#
# GNU
#

INCLUDE(CheckCCompilerFlag)
INCLUDE(CheckCXXCompilerFlag)

MACRO(_MOO_CHECK_COMPILER_FLAGS lang)
  _MOO_CHECK_COMPILER_LANG(${lang} _moo__ccf_lang)
  FOREACH(_moo__ccf_flag ${ARGN})
    STRING(REGEX REPLACE "[-=]" "_" _moo__ccf_flag_name MOO_${_moo__ccf_lang}FLAG${_moo__ccf_flag})
    STRING(REPLACE "+" "x" _moo__ccf_flag_name ${_moo__ccf_flag_name})
    IF(_moo__ccf_lang STREQUAL C)
      CHECK_C_COMPILER_FLAG("${_moo__ccf_flag}" ${_moo__ccf_flag_name})
    ELSEIF(_moo__ccf_lang STREQUAL CXX)
      CHECK_CXX_COMPILER_FLAG("${_moo__ccf_flag}" ${_moo__ccf_flag_name})
    ELSE(_moo__ccf_lang STREQUAL C)
      MOO_ERROR("Unknown lang '${lang}'")
    ENDIF(_moo__ccf_lang STREQUAL C)
    IF(${_moo__ccf_flag_name})
      SET(CMAKE_${_moo__ccf_lang}_FLAGS "${CMAKE_${_moo__ccf_lang}_FLAGS} ${_moo__ccf_flag}")
    ENDIF(${_moo__ccf_flag_name})
  ENDFOREACH(_moo__ccf_flag)
ENDMACRO(_MOO_CHECK_COMPILER_FLAGS)

IF(CMAKE_COMPILER_IS_GNUCC)
  _MOO_CHECK_COMPILER_FLAGS(C -Wall -Wextra -fno-strict-aliasing -Wno-missing-field-initializers)
ENDIF(CMAKE_COMPILER_IS_GNUCC)

IF(CMAKE_COMPILER_IS_GNUCXX)
  _MOO_CHECK_COMPILER_FLAGS(CXX -Wall -Wextra -fno-strict-aliasing -fno-exceptions -std=c++98 -pedantic -Wno-long-long)
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

###############################################################################
#
# Windows
#

IF(WIN32)
  ADD_DEFINITIONS(-D__WIN32__ -DWIN32_LEAN_AND_MEAN -DUNICODE -D_UNICODE)
ENDIF(WIN32)

IF(MSVC)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:wchar_t- /W4")
  FOREACH(_moo_w 4996 4244 4055 4127 4100 4054 4152 4706 4125 4389 4132 4018)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd${_moo_w}")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd${_moo_w}")
  ENDFOREACH(_moo_w)
ENDIF(MSVC)

###############################################################################
#
# Mac OS X
#

IF(APPLE)
  MOO_OPTION(MOO_ENABLE_UNIVERSAL FALSE "Build a universal binary on Mac OS X" HIDDEN !${MOO_OS_DARWIN})
ENDIF(APPLE)

IF(APPLE AND MOO_ENABLE_UNIVERSAL)
  SET(CMAKE_OSX_ARCHITECTURES "ppc;i386")
  SET(CMAKE_OSX_SYSROOT "/Developer/SDKs/MacOSX10.4u.sdk")
ENDIF(APPLE AND MOO_ENABLE_UNIVERSAL)


###############################################################################
#
# Precompiled headers
#

MOO_OPTION(MOO_ENABLE_PCH FALSE "Enable precompiled headers. May be broken.")

MACRO(__MOO_PCH_GNUCXX_GET_COMPILE_FLAGS outvar target)

  STRING(TOUPPER "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}" _moo_varname)
  LIST(APPEND ${outvar} ${${_moo_varname}})

  GET_TARGET_PROPERTY(target_type ${target} TYPE)
  IF(target_type STREQUAL SHARED_LIBRARY)
    LIST(APPEND ${outvar} -fPIC)
  ENDIF(target_type STREQUAL SHARED_LIBRARY)

  GET_DIRECTORY_PROPERTY(includes INCLUDE_DIRECTORIES)
  FOREACH(item ${includes})
    LIST(APPEND ${outvar} "-I${item}")
  ENDFOREACH(item)

  STRING(TOUPPER "COMPILE_DEFINITIONS_${CMAKE_BUILD_TYPE}" _moo_varname)
  GET_DIRECTORY_PROPERTY(flags ${_moo_varname})
  FOREACH(item ${flags})
    LIST(APPEND ${outvar} "-D${item}")
  ENDFOREACH(item)
  GET_DIRECTORY_PROPERTY(flags COMPILE_DEFINITIONS)
  FOREACH(item ${flags})
    LIST(APPEND ${outvar} "-D${item}")
  ENDFOREACH(item)

  GET_DIRECTORY_PROPERTY(flags DEFINITIONS)
  LIST(APPEND ${outvar} ${flags})
  LIST(APPEND ${outvar} ${CMAKE_CXX_FLAGS} )

  SEPARATE_ARGUMENTS(${outvar})
ENDMACRO(__MOO_PCH_GNUCXX_GET_COMPILE_FLAGS)

MACRO(__MOO_PCH_GNUCXX_GET_COMMAND outvar target input output)
  __MOO_PCH_GNUCXX_GET_COMPILE_FLAGS(_moo_compile_flags target)
  SET(_moo_pch_compile_command
    ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1} ${_moo_compile_flags} -x c++-header -o ${output} ${input}
  )
ENDMACRO(__MOO_PCH_GNUCXX_GET_COMMAND)

MACRO(__MOO_PRECOMPILE_HEADER_GNUCXX header target)
  SET(_moo_pch_output_dir ${CMAKE_CURRENT_BINARY_DIR}/${header}.gch)
  SET(_moo_pch_output ${_moo_pch_output_dir}/c++)
  SET(_moo_pch_input_s ${CMAKE_CURRENT_SOURCE_DIR}/${header})
  SET(_moo_pch_input_b ${CMAKE_CURRENT_BINARY_DIR}/${header})
  SET(_moo_pch_lib ${target}_pchlib)

  SET_SOURCE_FILES_PROPERTIES(${_moo_pch_input_b} PROPERTIES GENERATED 1)

  MOO_SUBDIR_NAME(_moo_subdir)

  ADD_CUSTOM_COMMAND(OUTPUT ${_moo_pch_input_b}
    COMMAND ${CMAKE_COMMAND} -E copy ${_moo_pch_input_s} ${_moo_pch_input_b}
    DEPENDS ${_moo_pch_input_s}
    COMMENT "Generating ${_moo_subdir}/${header}"
  )

  ADD_CUSTOM_TARGET(${target}_pch DEPENDS ${_moo_pch_output})
  ADD_DEPENDENCIES(${target} ${target}_pch)

  __MOO_PCH_GNUCXX_GET_COMMAND(_moo_pch_compile_command ${target} ${_moo_pch_input_b} ${_moo_pch_output})
  ADD_CUSTOM_COMMAND(OUTPUT ${_moo_pch_output}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${_moo_pch_output_dir}
    COMMAND ${_moo_pch_compile_command}
    DEPENDS ${_moo_pch_input_s} ${_moo_pch_input_b} # ${_moo_pch_lib}
    COMMENT "Generating ${_moo_subdir}/${header}.gch/c++"
  )

  GET_TARGET_PROPERTY(_moo_old_flags ${target} COMPILE_FLAGS)
  IF(NOT _moo_old_flags)
    SET(_moo_old_flags)
  ENDIF(NOT _moo_old_flags)
  SET_TARGET_PROPERTIES(${target} PROPERTIES COMPILE_FLAGS
    "-include ${_moo_pch_input_b} -Winvalid-pch ${_moo_old_flags}"
  )
ENDMACRO(__MOO_PRECOMPILE_HEADER_GNUCXX)

MACRO(MOO_PRECOMPILE_HEADER header target)
  IF(MOO_ENABLE_PCH)
    IF(CMAKE_COMPILER_IS_GNUCXX)
      __MOO_PRECOMPILE_HEADER_GNUCXX(${header} ${target})
    ELSE(CMAKE_COMPILER_IS_GNUCXX)
      MESSAGE(STATUS "*** IMPLEMENT ME: precompiled headers for this compiler")
    ENDIF(CMAKE_COMPILER_IS_GNUCXX)
  ENDIF(MOO_ENABLE_PCH)
ENDMACRO(MOO_PRECOMPILE_HEADER)
