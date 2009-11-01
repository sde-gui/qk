###########################################################################
#
# Common
#

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release)
ENDIF(NOT CMAKE_BUILD_TYPE)

SET(MOO_DEBUG OFF)
STRING(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
IF(CMAKE_BUILD_TYPE STREQUAL DEBUG)
  SET(MOO_DEBUG ON)
ENDIF(CMAKE_BUILD_TYPE STREQUAL DEBUG)


###########################################################################
#
# OS
#

IF(APPLE)
  SET(MOO_OS_DARWIN 1)
ENDIF(APPLE)

IF(CYGWIN)
  SET(MOO_OS_CYGWIN 1)
ENDIF(CYGWIN)

IF(WIN32 OR CYGWIN)
  SET(MOO_OS_WIN32 1)
ENDIF(WIN32 OR CYGWIN)

IF(UNIX)
  SET(MOO_OS_UNIX 1)
ENDIF(UNIX)

IF(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
  SET(MOO_OS_FREEBSD 1)
ENDIF(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")

IF(CMAKE_SYSTEM_NAME MATCHES "BSD")
  SET(MOO_OS_BSD 1)
ENDIF(CMAKE_SYSTEM_NAME MATCHES "BSD")

IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
  SET(MOO_OS_LINUX 1)
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Linux")

IF(MOO_OS_DARWIN OR MOO_OS_FREEBSD)
  SET(MOO_OS_BSD 1)
ENDIF(MOO_OS_DARWIN OR MOO_OS_FREEBSD)

IF(MOO_OS_BSD OR MOO_OS_LINUX)
  SET(MOO_OS_UNIX 1)
ENDIF(MOO_OS_BSD OR MOO_OS_LINUX)

FOREACH(_moo_os_name CYGWIN WIN32 DARWIN UNIX FREEBSD BSD LINUX)
  MOO_DEFINE_H(MOO_OS_${_moo_os_name})
ENDFOREACH(_moo_os_name)


###########################################################################
#
# Gnu
#

IF(CMAKE_COMPILER_IS_GNUCXX)
  INCLUDE(CheckCXXCompilerFlag)
  FOREACH(_moo_flag -Wall -Wextra -fno-strict-aliasing -fno-exceptions -std=c++98 -pedantic -Wno-long-long)
    STRING(REGEX REPLACE "[-=]" "_" _moo_flag_name MOO_CXXFLAG${_moo_flag})
    STRING(REPLACE "+" "x" _moo_flag_name ${_moo_flag_name})
    CHECK_CXX_COMPILER_FLAG("${_moo_flag}" ${_moo_flag_name})
    IF(${_moo_flag_name})
      SET(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${_moo_flag}")
    ENDIF(${_moo_flag_name})
  ENDFOREACH(_moo_flag)
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

IF(CMAKE_COMPILER_IS_GNUCC)
  INCLUDE(CheckCCompilerFlag)
  FOREACH(_moo_flag -Wall -Wextra -fno-strict-aliasing -Wno-missing-field-initializers)
    STRING(REPLACE "-" "_" _moo_flag_name MOO_CFLAG${_moo_flag})
    CHECK_C_COMPILER_FLAG("${_moo_flag}" ${_moo_flag_name})
    IF(${_moo_flag_name})
      SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${_moo_flag}")
    ENDIF(${_moo_flag_name})
  ENDFOREACH(_moo_flag)
ENDIF(CMAKE_COMPILER_IS_GNUCC)


###########################################################################
#
# Win32
#

IF(WIN32)
  ADD_DEFINITIONS(-D__WIN32__ -DWIN32_LEAN_AND_MEAN -DUNICODE -D_UNICODE)
ENDIF(WIN32)

IF(WIN32 AND CMAKE_COMPILER_IS_GNUCC)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mms-bitfields")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mms-bitfields")
ENDIF(WIN32 AND CMAKE_COMPILER_IS_GNUCC)

IF(MSVC)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4996")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996 /Zc:wchar_t-")
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS)
ENDIF(MSVC)


###############################################################################
#
# Mac OS X
#

IF(APPLE)
  SET(MOO_ENABLE_UNIVERSAL OFF CACHE BOOL "Set to build a universal binary on Mac OS X")
ENDIF(APPLE)

IF(APPLE AND MOO_ENABLE_UNIVERSAL)
  SET(CMAKE_OSX_ARCHITECTURES "ppc;i386")
  SET(CMAKE_OSX_SYSROOT "/Developer/SDKs/MacOSX10.4u.sdk")
ENDIF(APPLE AND MOO_ENABLE_UNIVERSAL)


###############################################################################
#
# Precompiled headers support
#

SET(MOO_ENABLE_PCH OFF CACHE BOOL "Set to enable precompiled headers. May be broken")

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
