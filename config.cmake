IF(GDK_X11)
  FIND_PACKAGE(X11)
  LIST(APPEND MOO_DEP_LIBS ${X11_LIBRARIES} ${X11_ICE_LIB})
ENDIF(GDK_X11)

# if test x$MOO_OS_DARWIN = xyes; then
#   _moo_ac_have_carbon=no
#   AC_MSG_CHECKING([for Mac OS X Carbon support])
#   AC_TRY_CPP([
#     #include <Carbon/Carbon.h>
#     #include <CoreServices/CoreServices.h>
#   ],[
#     _moo_ac_have_carbon=yes
#     AC_DEFINE(HAVE_CARBON, 1, [Mac OS X Carbon])
#     LDFLAGS="$LDFLAGS -framework Carbon"
#   ])
#   AC_MSG_RESULT([$_moo_ac_have_carbon])
# fi

# if $GDK_QUARTZ; then
#   PKG_CHECK_MODULES(IGE_MAC,ige-mac-integration)
#   GTK_CFLAGS="$IGE_MAC_CFLAGS"
#   GTK_LIBS="$IGE_MAC_LIBS"
#   LDFLAGS="$LDFLAGS -framework Cocoa"
# fi


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
# for mdfileops.c
MOO_CHECK_FUNCTIONS(link fchown fchmod)
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

IF(WIN32 AND NOT MSVC)
  FIND_PROGRAM(WINDRES_EXECUTABLE windres
               DOC "Path to the windres executable")
  IF(NOT WINDRES_EXECUTABLE)
    MESSAGE(FATAL_ERROR "Could not find windres")
  ENDIF(NOT WINDRES_EXECUTABLE)
ENDIF(WIN32 AND NOT MSVC)

# dnl _MOO_AC_CHECK_TOOL(variable,program)
# AC_DEFUN([_MOO_AC_CHECK_TOOL],[
#   AC_ARG_VAR([$1], [$2 program])
#   AC_PATH_PROG([$1], [$2], [])
#   AM_CONDITIONAL([HAVE_$1],[ test "x$2" != "x" ])
# ])
#
# AC_DEFUN_ONCE([MOO_AC_CHECK_TOOLS],[
#   _MOO_AC_CHECK_TOOL([GDK_PIXBUF_CSOURCE], [gdk-pixbuf-csource])
#   _MOO_AC_CHECK_TOOL([], [glib-genmarshal])
#   _MOO_AC_CHECK_TOOL([], [glib-mkenums])
#   _MOO_AC_CHECK_TOOL([TXT2TAGS], [txt2tags])
# ])
