OPTION(MOO_DEV_MODE_CMAKE "Enable developer mode for cmake - bunch of checks, warnings, etc." OFF)
MARK_AS_ADVANCED(MOO_DEV_MODE_CMAKE)

IF(MOO_DEV_MODE_CMAKE)
  MACRO(MOO_DEBUG)
    MESSAGE(${ARGN})
  ENDMACRO(MOO_DEBUG)
ELSE(MOO_DEV_MODE_CMAKE)
  MACRO(MOO_DEBUG)
  ENDMACRO(MOO_DEBUG)
ENDIF(MOO_DEV_MODE_CMAKE)

MACRO(MOO_ERROR)
  MESSAGE(FATAL_ERROR ${ARGN})
ENDMACRO(MOO_ERROR)

###########################################################################
#
# Build type
#

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
  SET(CMAKE_BUILD_TYPE Release)
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
STRING(TOUPPER ${CMAKE_BUILD_TYPE} MOO_BUILD_TYPE)
SET(MOO_BUILD_SUBTYPE ${MOO_BUILD_TYPE})
IF("${MOO_BUILD_TYPE}" STREQUAL "RELWITHDEBINFO" OR "${MOO_BUILD_TYPE}" STREQUAL "MINSIZEREL")
  SET(MOO_BUILD_TYPE RELEASE)
ELSEIF(NOT "${MOO_BUILD_TYPE}" STREQUAL "DEBUG" AND NOT "${MOO_BUILD_TYPE}" STREQUAL "RELEASE")
  MOO_ERROR("Unknown build type '${MOO_BUILD_TYPE}'")
ENDIF("${MOO_BUILD_TYPE}" STREQUAL "RELWITHDEBINFO" OR "${MOO_BUILD_TYPE}" STREQUAL "MINSIZEREL")
MOO_DEBUG(STATUS "MOO_BUILD_TYPE: ${MOO_BUILD_TYPE}")

IF("${MOO_BUILD_TYPE}" STREQUAL "DEBUG")
  SET(MOO_DEBUG TRUE)
ELSE("${MOO_BUILD_TYPE}" STREQUAL "DEBUG")
  SET(MOO_DEBUG FALSE)
ENDIF("${MOO_BUILD_TYPE}" STREQUAL "DEBUG")

###########################################################################
#
# MOO_DEFINE_H
#

SET(__MOO_DEFINE_H_FILE__ ${CMAKE_BINARY_DIR}/moo-config.h.in)
FILE(WRITE ${__MOO_DEFINE_H_FILE__} "")

MACRO(MOO_DEFINE_H _moo_varname)
  SET(_moo_comment)
  FOREACH(_moo_arg ${ARGN})
    SET(_moo_comment ${_moo_arg})
  ENDFOREACH(_moo_arg)
  IF(_moo_comment)
    FILE(APPEND ${__MOO_DEFINE_H_FILE__} "/* ${_moo_comment} */\n")
  ENDIF(_moo_comment)
  FILE(APPEND ${__MOO_DEFINE_H_FILE__} "#moodefine ${_moo_varname}\n\n")
ENDMACRO(MOO_DEFINE_H)

MACRO(MOO_MAKE_ABSOLUTE_INPUT_FILE file out_var)
  SET(_moo_maif_abs_file "${file}")
  IF(NOT EXISTS "${_moo_maif_abs_file}")
    IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_moo_maif_abs_file}")
      SET(_moo_maif_abs_file ${CMAKE_CURRENT_SOURCE_DIR}/${_moo_maif_abs_file})
    ENDIF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_moo_maif_abs_file}")
  ENDIF(NOT EXISTS "${_moo_maif_abs_file}")
  IF(NOT EXISTS "${_moo_maif_abs_file}")
    MOO_ERROR("File '${_moo_maif_abs_file}' does not exist")
  ENDIF(NOT EXISTS "${_moo_maif_abs_file}")
  SET(${out_var} "${_moo_maif_abs_file}")
ENDMACRO(MOO_MAKE_ABSOLUTE_INPUT_FILE)

MACRO(MOO_MAKE_ABSOLUTE_OUTPUT_FILE file out_var)
  SET(_moo_maof_abs_file "${file}")
  IF(NOT IS_ABSOLUTE "${_moo_maof_abs_file}")
    SET(_moo_maof_abs_file "${CMAKE_CURRENT_BINARY_DIR}/${_moo_maof_abs_file}")
  ENDIF(NOT IS_ABSOLUTE "${_moo_maof_abs_file}")
  SET(${out_var} "${_moo_maof_abs_file}")
ENDMACRO(MOO_MAKE_ABSOLUTE_OUTPUT_FILE)

MACRO(MOO_WRITE_CONFIG_H _moo_in_file _moo_out_file)
  MOO_MAKE_ABSOLUTE_INPUT_FILE("${_moo_in_file}" _moo_abs_in_file)
  MOO_MAKE_ABSOLUTE_OUTPUT_FILE("${_moo_out_file}" _moo_abs_out_file)

  FILE(WRITE ${_moo_abs_out_file}-in.moo "#ifndef __CONFIG_H__\n")
  FILE(APPEND ${_moo_abs_out_file}-in.moo "#define __CONFIG_H__\n\n")
  FILE(READ ${_moo_abs_in_file} _moo_contents)
  FILE(APPEND ${_moo_abs_out_file}-in.moo ${_moo_contents})
  FILE(APPEND ${_moo_abs_out_file}-in.moo "\n")

  FILE(STRINGS ${__MOO_DEFINE_H_FILE__} _moo_wch_lines)
  FOREACH(_moo_wch_l ${_moo_wch_lines})
    IF(_moo_wch_l MATCHES "^#moodefine (.*)$")
      STRING(REGEX REPLACE "^#moodefine (.*)$" "\\1" _moo_wch_var "${_moo_wch_l}")
      IF(${_moo_wch_var})
        FILE(APPEND ${_moo_abs_out_file}-in.moo "#define ${_moo_wch_var} 1\n\n")
      ELSE(${_moo_wch_var})
        FILE(APPEND ${_moo_abs_out_file}-in.moo "/* #undef ${_moo_wch_var} */\n\n")
      ENDIF(${_moo_wch_var})
    ELSE(_moo_wch_l MATCHES "^#moodefine (.*)$")
      FILE(APPEND ${_moo_abs_out_file}-in.moo "${_moo_wch_l}\n")
    ENDIF(_moo_wch_l MATCHES "^#moodefine (.*)$")
  ENDFOREACH(_moo_wch_l)

  FILE(APPEND ${_moo_abs_out_file}-in.moo "#endif /* __CONFIG_H__ */\n")

  CONFIGURE_FILE(${_moo_abs_out_file}-in.moo ${_moo_abs_out_file} ${ARGN})
ENDMACRO(MOO_WRITE_CONFIG_H)

###########################################################################
#
# OS
#

SET(_MOO_KNOWN_PLATFORMS CYGWIN WIN32 DARWIN UNIX FREEBSD BSD LINUX FDO)
FOREACH(_moo_os ${_MOO_KNOWN_PLATFORMS})
  SET(MOO_OS_${_moo_os} FALSE)
ENDFOREACH(_moo_os)

IF(APPLE)
  SET(MOO_OS_DARWIN TRUE)
ENDIF(APPLE)

IF(CYGWIN)
  SET(MOO_OS_CYGWIN TRUE)
ENDIF(CYGWIN)

IF(WIN32 OR CYGWIN)
  SET(MOO_OS_WIN32 TRUE)
ENDIF(WIN32 OR CYGWIN)

IF(UNIX)
  SET(MOO_OS_UNIX TRUE)
ENDIF(UNIX)

IF(UNIX AND NOT WIN32 AND NOT APPLE)
  SET(MOO_OS_FDO TRUE)
ENDIF(UNIX AND NOT WIN32 AND NOT APPLE)

IF(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
  SET(MOO_OS_FREEBSD TRUE)
ENDIF(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")

IF(CMAKE_SYSTEM_NAME MATCHES "BSD")
  SET(MOO_OS_BSD TRUE)
ENDIF(CMAKE_SYSTEM_NAME MATCHES "BSD")

IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
  SET(MOO_OS_LINUX TRUE)
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Linux")

IF(MOO_OS_DARWIN OR MOO_OS_FREEBSD)
  SET(MOO_OS_BSD TRUE)
ENDIF(MOO_OS_DARWIN OR MOO_OS_FREEBSD)

IF(MOO_OS_BSD OR MOO_OS_LINUX)
  SET(MOO_OS_UNIX TRUE)
ENDIF(MOO_OS_BSD OR MOO_OS_LINUX)

###########################################################################
#
# Installation dirs
#

SET(BINDIR bin CACHE PATH "bin")
SET(DATADIR share CACHE PATH "share")
SET(DESKTOPFILEDIR ${DATADIR}/applications CACHE PATH "share/applications")
SET(LIBDIR lib CACHE PATH "lib")
SET(LOCALEDIR ${DATADIR}/locale CACHE PATH "Where mo files go")

###########################################################################
#
# Aux macros
#

# MOO_COLLECT_ARGS(ARG FOO VAR foo ARG BAR VAR bar ARGN ...)
# caller: SOME_MACRO(FOO foo1 foo2 foo3 BAR bar1)
MACRO(MOO_COLLECT_ARGS)
  SET(_moo_args)
  SET(_moo_vars)
  SET(_moo_argn)

  SET(_moo_what)

  FOREACH(_moo_a ${ARGN})
    IF("${_moo_what}" STREQUAL "ARG")
      LIST(APPEND _moo_args ${_moo_a})
      SET(_moo_what VAR)
    ELSEIF("${_moo_what}" STREQUAL "VAR")
      LIST(APPEND _moo_vars ${_moo_a})
      SET(_moo_what)
    ELSEIF("${_moo_what}" STREQUAL "ARGN")
      LIST(APPEND _moo_argn ${_moo_a})
    ELSEIF("${_moo_a}" STREQUAL "ARG")
      SET(_moo_what ARG)
    ELSEIF("${_moo_a}" STREQUAL "ARGN")
      SET(_moo_what ARGN)
    ELSE("${_moo_what}" STREQUAL "ARG")
      MOO_ERROR("bad argument ${moo_a}")
    ENDIF("${_moo_what}" STREQUAL "ARG")
  ENDFOREACH(_moo_a)

#   MESSAGE("args: ${_moo_args}")
#   MESSAGE("vars: ${_moo_vars}")
#   MESSAGE("argn: ${_moo_argn}")

  SET(_moo_what -1)
  FOREACH(_moo_arg ${_moo_argn})
    LIST(FIND _moo_args "${_moo_arg}" _moo_what2)
    IF(NOT _moo_what2 EQUAL -1)
      SET(_moo_what ${_moo_what2})
    ELSE(NOT _moo_what2 EQUAL -1)
      IF(_moo_what EQUAL -1)
        MOO_ERROR("Invalid argument ${_moo_arg}")
      ENDIF(_moo_what EQUAL -1)
      LIST(GET _moo_vars ${_moo_what} _moo_v)
      LIST(APPEND ${_moo_v} "${_moo_arg}")
    ENDIF(NOT _moo_what2 EQUAL -1)
  ENDFOREACH(_moo_arg)

  LIST(LENGTH _moo_args _moo_nargs)
  MATH(EXPR _moo_nargs "${_moo_nargs} - 1")
  FOREACH(_moo_what RANGE ${_moo_nargs})
    LIST(GET _moo_args ${_moo_what} _moo_a)
    LIST(GET _moo_vars ${_moo_what} _moo_v)
    #MOO_DEBUG("${_moo_what}. ${_moo_v}: ${${_moo_v}}")
  ENDFOREACH(_moo_what)
ENDMACRO(MOO_COLLECT_ARGS)

MACRO(__MOO_SET_SIMPLE_VAR var)
  SET(${var} ${ARGN})
ENDMACRO(__MOO_SET_SIMPLE_VAR)

# _MOO_COLLECT_ARGS_IMPL(FOO foo setfoo BAR bar setbar ARGN ...)
MACRO(_MOO_COLLECT_ARGS_IMPL)
  SET(_moo__cai_args)
  SET(_moo__cai_vars)
  SET(_moo__cai_macros)
  SET(_moo__cai_argn)

  SET(_moo__cai_what)

  FOREACH(_moo__cai_a ${ARGN})
    IF("${_moo__cai_what}" STREQUAL "VAR")
      LIST(APPEND _moo__cai_vars ${_moo__cai_a})
      SET(_moo__cai_what SETVAR)
    ELSEIF("${_moo__cai_what}" STREQUAL "SETVAR")
      LIST(APPEND _moo__cai_macros ${_moo__cai_a})
      SET(_moo__cai_what)
    ELSEIF("${_moo__cai_what}" STREQUAL "ARGN")
      LIST(APPEND _moo__cai_argn ${_moo__cai_a})
    ELSEIF("${_moo__cai_a}" STREQUAL "ARGN")
      SET(_moo__cai_what ARGN)
    ELSE("${_moo__cai_what}" STREQUAL "VAR")
      LIST(APPEND _moo__cai_args ${_moo__cai_a})
      SET(_moo__cai_what VAR)
    ENDIF("${_moo__cai_what}" STREQUAL "VAR")
  ENDFOREACH(_moo__cai_a)

#   MOO_DEBUG("args: ${_moo__cai_args}")
#   MOO_DEBUG("vars: ${_moo__cai_vars}")
#   MOO_DEBUG("macros: ${_moo__cai_macros}")
#   MOO_DEBUG("argn: ${_moo__cai_argn}")

  SET(_moo__cai_what -1)
  FOREACH(_moo__cai_arg ${_moo__cai_argn})
    LIST(FIND _moo__cai_args "${_moo__cai_arg}" _moo__cai_what2)
    IF(NOT _moo__cai_what2 EQUAL -1)
      SET(_moo__cai_what ${_moo__cai_what2})
    ELSE(NOT _moo__cai_what2 EQUAL -1)
      IF(_moo__cai_what EQUAL -1)
        MOO_ERROR("Invalid argument ${_moo__cai_arg}")
      ENDIF(_moo__cai_what EQUAL -1)
      LIST(GET _moo__cai_vars ${_moo__cai_what} _moo__cai_v)
      LIST(GET _moo__cai_macros ${_moo__cai_what} _moo__cai_sv)
      IF("${_moo__cai_sv}" STREQUAL "__MOO_SET_SIMPLE_VAR")
#         MOO_DEBUG("${_moo__cai_v} = ${_moo__cai_arg}")
        __MOO_SET_SIMPLE_VAR(${_moo__cai_v} ${_moo__cai_arg})
      ELSE("${_moo__cai_sv}" STREQUAL "__MOO_SET_SIMPLE_VAR")
        MOO_ERROR("Invalid argument ${_moo__cai_sv}")
      ENDIF("${_moo__cai_sv}" STREQUAL "__MOO_SET_SIMPLE_VAR")
    ENDIF(NOT _moo__cai_what2 EQUAL -1)
  ENDFOREACH(_moo__cai_arg)

  LIST(LENGTH _moo__cai_args _moo__cai_nargs)
  MATH(EXPR _moo__cai_nargs "${_moo__cai_nargs} - 1")
  FOREACH(_moo__cai_what RANGE ${_moo__cai_nargs})
    LIST(GET _moo__cai_args ${_moo__cai_what} _moo__cai_a)
    LIST(GET _moo__cai_vars ${_moo__cai_what} _moo__cai_v)
#     MOO_DEBUG("${_moo__cai_what}. ${_moo__cai_v}: ${${_moo__cai_v}}")
  ENDFOREACH(_moo__cai_what)
ENDMACRO(_MOO_COLLECT_ARGS_IMPL)

# MOO_COLLECT_SIMPLE_ARGS(FOO foo BAR bar ARGN ...)
MACRO(MOO_COLLECT_SIMPLE_ARGS)
  SET(_moo_csa_impl_argn)

  SET(_moo_csa_argn ${ARGN})
  SET(_moo_csa_i 0)
  LIST(LENGTH _moo_csa_argn _moo_csa_nargs)
  WHILE(_moo_csa_i LESS _moo_csa_nargs)
    LIST(GET _moo_csa_argn ${_moo_csa_i} _moo_csa_a)
    IF("${_moo_csa_a}" STREQUAL "ARGN")
      BREAK()
    ENDIF("${_moo_csa_a}" STREQUAL "ARGN")
    MATH(EXPR _moo_csa_i "${_moo_csa_i} + 1")
    LIST(GET _moo_csa_argn ${_moo_csa_i} _moo_csa_v)
    LIST(APPEND _moo_csa_impl_argn "${_moo_csa_a}")
    LIST(APPEND _moo_csa_impl_argn "${_moo_csa_v}")
    LIST(APPEND _moo_csa_impl_argn __MOO_SET_SIMPLE_VAR)
    MATH(EXPR _moo_csa_i "${_moo_csa_i} + 1")
  ENDWHILE(_moo_csa_i LESS _moo_csa_nargs)

  WHILE(_moo_csa_i LESS _moo_csa_nargs)
    LIST(GET _moo_csa_argn ${_moo_csa_i} _moo_csa_a)
    LIST(APPEND _moo_csa_impl_argn "${_moo_csa_a}")
    MATH(EXPR _moo_csa_i "${_moo_csa_i} + 1")
  ENDWHILE(_moo_csa_i LESS _moo_csa_nargs)

#   MOO_DEBUG("MOO_COLLECT_SIMPLE_ARGS: ${_moo_csa_argn}")
#   MOO_DEBUG("MOO_COLLECT_SIMPLE_ARGS: ${_moo_csa_impl_argn}")

  _MOO_COLLECT_ARGS_IMPL(${_moo_csa_impl_argn})
ENDMACRO(MOO_COLLECT_SIMPLE_ARGS)

###########################################################################
#
# MOO_OPTION
#

MACRO(MOO_OPTION variable dfltval helpstring)
  IF(DEFINED __MOO_OPTION_SET_${variable})
    MOO_ERROR("Option ${variable} is already created")
  ENDIF(DEFINED __MOO_OPTION_SET_${variable})
  SET(_moo_option_hidden)
  SET(_moo_option_header)
  SET(_moo_option_define)
  MOO_COLLECT_SIMPLE_ARGS(HIDDEN _moo_option_hidden HEADER _moo_option_header DEFINE _moo_option_define ARGN ${ARGN})
  IF(NOT _moo_option_hidden)
    OPTION(${variable} "${helpstring}" "${dfltval}")
  ELSE(NOT _moo_option_hidden)
    SET(${variable} "${dfltval}" CACHE BOOL "${helpstring}")
    MARK_AS_ADVANCED(${variable})
  ENDIF(NOT _moo_option_hidden)
  IF(_moo_option_header)
#     MOO_DEBUG(${variable})
    MOO_DEFINE_H(${variable} "${helpstring}")
  ENDIF(_moo_option_header)
  IF(_moo_option_define AND ${variable})
    ADD_DEFINITIONS(-D${variable}=1)
  ENDIF(_moo_option_define AND ${variable})
  SET(__MOO_OPTION_SET_${variable} 1)
#   MOO_DEBUG(STATUS "Option ${variable}")
ENDMACRO(MOO_OPTION)

MACRO(MOO_CHECK_OPTION variable)
  IF(NOT DEFINED __MOO_OPTION_SET_${variable})
    MOO_ERROR("Option ${variable} is not set")
  ENDIF(NOT DEFINED __MOO_OPTION_SET_${variable})
ENDMACRO(MOO_CHECK_OPTION)

MOO_OPTION(MOO_DEV_MODE FALSE "Enable developer mode - bunch of checks, warnings, etc." DEFINE 1)

FIND_PACKAGE(MooCompilers)
FIND_PACKAGE(MooCmakeUtils)
FIND_PACKAGE(MooI18n)
