IF(GDK_X11)
  FIND_PACKAGE(X11)
  LIST(APPEND MOO_DEP_LIBS ${X11_LIBRARIES} ${X11_ICE_LIB})
ENDIF(GDK_X11)

INCLUDE(CheckFunctionExists)
INCLUDE(CheckIncludeFile)
INCLUDE(CheckStructHasMember)

MACRO(MOO_CHECK_FUNCTIONS)
  FOREACH(_moo_func ${ARGN})
    STRING(REGEX REPLACE "[/.-]" "_" _moo_func_var ${_moo_func})
    STRING(TOUPPER HAVE_${_moo_func_var} _moo_func_var)
    CHECK_FUNCTION_EXISTS(${_moo_func} ${_moo_func_var})
    MOO_DEFINE_H(${_moo_func_var})
  ENDFOREACH(_moo_func)
ENDMACRO(MOO_CHECK_FUNCTIONS)

MACRO(MOO_CHECK_HEADERS)
  FOREACH(_moo_func ${ARGN})
    STRING(REGEX REPLACE "[/.-]" "_" _moo_func_var ${_moo_func})
    STRING(TOUPPER HAVE_${_moo_func_var} _moo_func_var)
    CHECK_INCLUDE_FILE(${_moo_func} ${_moo_func_var})
    MOO_DEFINE_H(${_moo_func_var})
  ENDFOREACH(_moo_func)
ENDMACRO(MOO_CHECK_HEADERS)

MACRO(MOO_CHECK_STRUCT_HAS_MEMBER struct field incfile varname)
  CHECK_STRUCT_HAS_MEMBER(${struct} ${field} ${incfile} ${varname})
  MOO_DEFINE_H(${varname})
ENDMACRO(MOO_CHECK_STRUCT_HAS_MEMBER)


# for xdgmime
MOO_CHECK_FUNCTIONS(getc_unlocked)
# for GMappedFile and xdgmime
MOO_CHECK_FUNCTIONS(mmap)
MOO_CHECK_HEADERS(unistd.h)
# for mooappabout.h
MOO_CHECK_HEADERS(sys/utsname.h)
# for mooapp.c
MOO_CHECK_HEADERS(signal.h)
MOO_CHECK_FUNCTIONS(signal)
# for mooutils-fs.c
MOO_CHECK_HEADERS(sys/wait.h)


IF(WIN32)
  INCLUDE_DIRECTORIES(${MOO_SOURCE_DIR}/moo/mooutils/moowin32/mingw)
  SET(HAVE_MMAP ON) # using fake mmap on windows
  # gettimeofday is present in recent mingw
  CHECK_FUNCTION_EXISTS(gettimeofday HAVE_GETTIMEOFDAY)
  IF(NOT HAVE_GETTIMEOFDAY)
    SET(HAVE_GETTIMEOFDAY 1)
    SET(MOO_NEED_GETTIMEOFDAY 1)
    INCLUDE_DIRECTORIES(${MOO_SOURCE_DIR}/moo/mooutils/moowin32/ms)
  ENDIF(NOT HAVE_GETTIMEOFDAY)
  MOO_DEFINE_H(MOO_NEED_GETTIMEOFDAY)
ENDIF(WIN32)

# -%- indent-width:2 -%-
