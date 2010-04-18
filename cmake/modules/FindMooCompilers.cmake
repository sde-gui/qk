MACRO(_MOO_JOIN_STRING_LIST outvar sep)
  SET(_moo__jsl_result)
  SET(_moo__jsl_first TRUE)
  FOREACH(_moo__jsl_arg ${ARGN})
    IF(_moo__jsl_first)
      SET(_moo__jsl_result "${_moo__jsl_arg}")
      SET(_moo__jsl_first FALSE)
    ELSE(_moo__jsl_first)
      SET(_moo__jsl_result "${_moo__jsl_result}${sep}${_moo__jsl_arg}")
    ENDIF(_moo__jsl_first)
  ENDFOREACH(_moo__jsl_arg)
  SET(${outvar} "${_moo__jsl_result}")
ENDMACRO(_MOO_JOIN_STRING_LIST)

MACRO(_MOO_CHECK_COMPILER_LANG lang var)
  STRING(TOUPPER ${lang} _moo__ccl_lang)
  IF("${_moo__ccl_lang}" STREQUAL "ALL")
    SET(_moo__ccl_langs C CXX)
  ELSEIF("${_moo__ccl_lang}" STREQUAL "C" OR "${_moo__ccl_lang}" STREQUAL "CXX")
    SET(_moo__ccl_langs ${_moo__ccl_lang})
  ELSE("${_moo__ccl_lang}" STREQUAL "ALL")
    MOO_ERROR("Invalid lang '${lang}'")
  ENDIF("${_moo__ccl_lang}" STREQUAL "ALL")
  SET(${var} ${_moo__ccl_langs})
ENDMACRO(_MOO_CHECK_COMPILER_LANG)

MACRO(_MOO_CHECK_BUILD_TYPE build prefix var)
  STRING(TOUPPER ${build} _moo__cbt_build)
  IF("${_moo__cbt_build}" STREQUAL "ALL")
    SET(_moo__cbt_vars ${prefix})
  ELSEIF("${_moo__cbt_build}" STREQUAL "RELEASE")
    SET(_moo__cbt_vars ${prefix}_RELEASE ${prefix}_RELWITHDEBINFO ${prefix}_MINSIZEREL)
  ELSEIF("${_moo__cbt_build}" STREQUAL "RELEASEONLY")
    SET(_moo__cbt_vars ${prefix}_RELEASE)
  ELSEIF("${_moo__cbt_build}" STREQUAL "DEBUG" OR "${_moo__cbt_build}" STREQUAL "RELWITHDEBINFO" OR "${_moo__cbt_build}" STREQUAL "MINSIZEREL")
    SET(_moo__cbt_vars ${prefix}_${_moo__cbt_build})
  ELSE("${_moo__cbt_build}" STREQUAL "ALL")
    MOO_ERROR("Invalid build type '${build}'")
  ENDIF("${_moo__cbt_build}" STREQUAL "ALL")
  SET(${var} ${_moo__cbt_vars})
ENDMACRO(_MOO_CHECK_BUILD_TYPE)

MACRO(_MOO_GET_COMPILER_CONFIG_ARGS var_lang var_build_prefix var_build_vars var_args)
  SET(_moo__gcca_langs C CXX)
  SET(_moo__gcca_args)
  SET(_moo__gcca_do TRUE)
  _MOO_CHECK_BUILD_TYPE("ALL" ${var_build_prefix} _moo__gcca_bvars)

  FOREACH(_moo__gcca_arg ${ARGN})
    IF("${_moo__gcca_arg}" STREQUAL "GCC" OR "${_moo__gcca_arg}" STREQUAL "GNUCC")
      IF(NOT CMAKE_COMPILER_IS_GNUCC)
        SET(_moo__gcca_do FALSE)
      ENDIF(NOT CMAKE_COMPILER_IS_GNUCC)
    ELSEIF("${_moo__gcca_arg}" STREQUAL "MSVC")
      IF(NOT MSVC)
        SET(_moo__gcca_do FALSE)
      ENDIF(NOT MSVC)
    ELSEIF("${_moo__gcca_arg}" STREQUAL "DEBUG" OR "${_moo__gcca_arg}" STREQUAL "RELEASE" OR
           "${_moo__gcca_arg}" STREQUAL "RELEASEONLY" OR "${_moo__gcca_arg}" STREQUAL "RELWITHDEBINFO" OR
           "${_moo__gcca_arg}" STREQUAL "MINSIZEREL")
      _MOO_CHECK_BUILD_TYPE(${_moo__gcca_arg} ${var_build_prefix} _moo__gcca_bvars)
    ELSEIF("${_moo__gcca_arg}" STREQUAL "C" OR "${_moo__gcca_arg}" STREQUAL "CXX")
      SET(_moo__gcca_langs ${_moo__gcca_arg})
    ELSEIF("${_moo__gcca_arg}" STREQUAL "WIN32" OR "${_moo__gcca_arg}" STREQUAL "UNIX")
      IF("${_moo__gcca_arg}" STREQUAL "WIN32" AND NOT WIN32)
	SET(_moo__gcca_do FALSE)
      ELSEIF("${_moo__gcca_arg}" STREQUAL "UNIX" AND WIN32)
	SET(_moo__gcca_do FALSE)
      ENDIF("${_moo__gcca_arg}" STREQUAL "WIN32" AND NOT WIN32)
    ELSE("${_moo__gcca_arg}" STREQUAL "GCC" OR "${_moo__gcca_arg}" STREQUAL "GNUCC")
      LIST(APPEND _moo__gcca_args ${_moo__gcca_arg})
    ENDIF("${_moo__gcca_arg}" STREQUAL "GCC" OR "${_moo__gcca_arg}" STREQUAL "GNUCC")
  ENDFOREACH(_moo__gcca_arg)

  #MESSAGE("_MOO_GET_COMPILER_CONFIG_ARGS(var_lang=${var_lang}, var_build_prefix=${var_build_prefix}, var_build_vars=${var_build_vars}, var_args=${var_args}, args=${ARGN})")
  IF(_moo__gcca_do)
    SET(${var_lang} ${_moo__gcca_langs})
    SET(${var_build_vars} ${_moo__gcca_bvars})
    SET(${var_args} ${_moo__gcca_args})
    #MESSAGE("    var_lang=${_moo__gcca_langs}, var_build_vars=${_moo__gcca_bsfxs}, var_args=${_moo__gcca_args}")
  ELSE(_moo__gcca_do)
    SET(${var_lang})
    SET(${var_build_vars})
    SET(${var_args})
    #MESSAGE("    DISABLED")
  ENDIF(_moo__gcca_do)
ENDMACRO(_MOO_GET_COMPILER_CONFIG_ARGS)

MACRO(MOO_ADD_COMPILER_FLAGS)
  _MOO_GET_COMPILER_CONFIG_ARGS(_moo_acf_langs "FLAGS" _moo_acf_bvars _moo_acf_args ${ARGN})
  IF(_moo_acf_args)
    _MOO_JOIN_STRING_LIST(_moo_acf_flags " " ${_moo_acf_args})
    FOREACH(_moo_acf_lang ${_moo_acf_langs})
      FOREACH(_moo_acf_bvar ${_moo_acf_bvars})
        SET(_moo__acf_var CMAKE_${_moo_acf_lang}_${_moo_acf_bvar})
        SET(${_moo__acf_var} "${${_moo__acf_var}} ${_moo_acf_flags}")
        MOO_DEBUG(STATUS "${_moo__acf_var} += ${_moo_acf_flags}")
      ENDFOREACH(_moo_acf_bvar)
    ENDFOREACH(_moo_acf_lang)
  ENDIF(_moo_acf_args)
ENDMACRO(MOO_ADD_COMPILER_FLAGS)


INCLUDE(CheckCCompilerFlag)
INCLUDE(CheckCXXCompilerFlag)

MACRO(MOO_COMPILER_FLAG_VAR_NAME flag lang var)
  STRING(REGEX REPLACE "[-=]" "_" _moo_cfvn_name MOO_${lang}FLAG${flag})
  STRING(REPLACE "+" "x" _moo_cfvn_name ${_moo_cfvn_name})
  SET(${var} ${_moo_cfvn_name})
ENDMACRO(MOO_COMPILER_FLAG_VAR_NAME)

MACRO(MOO_CHECK_COMPILER_FLAGS)
  _MOO_GET_COMPILER_CONFIG_ARGS(_moo_ccf_langs "FLAGS" _moo_ccf_bvars _moo_ccf_args ${ARGN})
  FOREACH(_moo_ccf_lang ${_moo_ccf_langs})
    SET(_moo_ccf_good_flags)
    FOREACH(_moo_ccf_flag ${_moo_ccf_args})
      MOO_COMPILER_FLAG_VAR_NAME(${_moo_ccf_flag} ${_moo_ccf_lang} _moo_ccf_flag_name)

      IF("${_moo_ccf_lang}" STREQUAL "C")
        CHECK_C_COMPILER_FLAG("${_moo_ccf_flag}" ${_moo_ccf_flag_name})
      ELSEIF("${_moo_ccf_lang}" STREQUAL "CXX")
        CHECK_CXX_COMPILER_FLAG("${_moo_ccf_flag}" ${_moo_ccf_flag_name})
      ELSE("${_moo_ccf_lang}" STREQUAL "C")
        MOO_ERROR("Unknown lang '${lang}'")
      ENDIF("${_moo_ccf_lang}" STREQUAL "C")

      IF(${_moo_ccf_flag_name})
        LIST(APPEND _moo_ccf_good_flags "${_moo_ccf_flag}")
      ENDIF(${_moo_ccf_flag_name})
    ENDFOREACH(_moo_ccf_flag)
    IF(_moo_ccf_good_flags)
      _MOO_JOIN_STRING_LIST(_moo_ccf_good_flags " " ${_moo_ccf_good_flags})
      FOREACH(_moo_ccf_bvar ${_moo_ccf_bvars})
        SET(_moo_ccf_flags_var CMAKE_${_moo_ccf_lang}_${_moo_ccf_bvar})
        SET(${_moo_ccf_flags_var} "${${_moo_ccf_flags_var}} ${_moo_ccf_good_flags}")
        MOO_DEBUG(STATUS "${_moo_ccf_flags_var} += ${_moo_ccf_good_flags}")
      ENDFOREACH(_moo_ccf_bvar)
    ENDIF(_moo_ccf_good_flags)
  ENDFOREACH(_moo_ccf_lang)
ENDMACRO(MOO_CHECK_COMPILER_FLAGS)


MACRO(MOO_ADD_COMPILE_DEFINITIONS)
  _MOO_GET_COMPILER_CONFIG_ARGS(_moo_acd_langs "COMPILE_DEFINITIONS" _moo_acd_bvars _moo_acd_args ${ARGN})
  SET(_moo_acd_defs)
  FOREACH(_moo_acd_a ${_moo_acd_args})
    STRING(REGEX REPLACE "^(-D|/D)" "" _moo_acd_a "${_moo_acd_a}")
    LIST(APPEND _moo_acd_defs "${_moo_acd_a}")
  ENDFOREACH(_moo_acd_a)
  #MESSAGE("MOO_ADD_COMPILE_DEFINITIONS(${ARGN}) => ${_moo_acd_langs}, ${_moo_acd_bvars}, ${_moo_acd_args}")
  FOREACH(_moo_acd_bvar ${_moo_acd_bvars})
    SET_PROPERTY(DIRECTORY APPEND PROPERTY ${_moo_acd_bvar} ${_moo_acd_defs})
    _MOO_JOIN_STRING_LIST(_moo_acd_defs_string " " ${_moo_acd_defs})
    MOO_DEBUG(STATUS "${_moo_acd_bvar} += ${_moo_acd_defs_string}")
  ENDFOREACH(_moo_acd_bvar)
ENDMACRO(MOO_ADD_COMPILE_DEFINITIONS)


###########################################################################
#
# DEBUG
#

MOO_ADD_COMPILE_DEFINITIONS(DEBUG -DMOO_DEBUG -DDEBUG)


###########################################################################
#
# GCC
#

# Set this first because it may affect checks below
IF(MOO_DEV_MODE)
  MOO_ADD_COMPILER_FLAGS(GCC -Werror)
ENDIF(MOO_DEV_MODE)

MOO_ADD_COMPILER_FLAGS(GCC -Wall -Wextra)
MOO_CHECK_COMPILER_FLAGS(GCC -fexceptions -fno-strict-aliasing -fno-strict-overflow)
MOO_CHECK_COMPILER_FLAGS(GCC -Wno-missing-field-initializers -Wno-overlength-strings  -Wno-missing-declarations)

MOO_CHECK_COMPILER_FLAGS(GCC DEBUG -ftrapv)

MOO_ADD_COMPILER_FLAGS(GCC CXX -std=c++98)
MOO_CHECK_COMPILER_FLAGS(GCC CXX -fno-rtti)
MOO_CHECK_COMPILER_FLAGS(GCC CXX RELEASE -fno-enforce-eh-specs)

IF(MOO_DEV_MODE)
  MOO_CHECK_COMPILER_FLAGS(GCC
    -Wcast-align -Wlogical-op
    -Wmissing-format-attribute -Wnested-externs -Wlong-long -Wvla
    -Wuninitialized -Winit-self)
  MOO_CHECK_COMPILER_FLAGS(GCC CXX -fno-nonansi-builtins -fno-gnu-keywords)
  MOO_CHECK_COMPILER_FLAGS(GCC CXX
    -Wctor-dtor-privacy -Wnon-virtual-dtor -Wabi
    -Wstrict-null-sentinel -Woverloaded-virtual -Wsign-promo
  )
ENDIF(MOO_DEV_MODE)


###############################################################################
#
# Windows
#

MOO_ADD_COMPILE_DEFINITIONS(WIN32 -D__WIN32__ -DWIN32_LEAN_AND_MEAN -DUNICODE -D_UNICODE -DSTRICT)

MOO_ADD_COMPILER_FLAGS(MSVC /W4 /GS)
MOO_ADD_COMPILER_FLAGS(MSVC CXX /Zc:wchar_t,forScope /GR /EHc /EHsc)

MOO_ADD_COMPILER_FLAGS(MSVC
  /wd4221 /wd4204 /wd4996 /wd4244 /wd4055 /wd4127 /wd4100
  /wd4054 /wd4152 /wd4706 /wd4125 /wd4389 /wd4132 /wd4018
)

IF(MOO_DEV_MODE)
  MOO_ADD_COMPILER_FLAGS(MSVC /WX)
  MOO_ADD_COMPILER_FLAGS(MSVC DEBUG /RTCsu /RTCc)
ENDIF(MOO_DEV_MODE)


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
  IF("${target_type}" STREQUAL "SHARED_LIBRARY")
    LIST(APPEND ${outvar} -fPIC)
  ENDIF("${target_type}" STREQUAL "SHARED_LIBRARY")

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
