###################################################################
# Library
#

SET(MOO_BUILD_SHARED_LIB ${WIN32} CACHE BOOL "Build shared library")

###################################################################
# comps
#

IF(NOT WIN32)
  SET(MOO_BUILD_CTAGS ON CACHE BOOL "Build Ctags plugin")
ELSE(NOT WIN32)
  SET(MOO_BUILD_CTAGS OFF)
  MARK_AS_ADVANCED(MOO_BUILD_CTAGS)
ENDIF(NOT WIN32)
MOO_DEFINE_H(MOO_BUILD_CTAGS "build ctags plugin")

SET(MOO_ENABLE_PROJECT OFF CACHE BOOL "Build project plugin")
MARK_AS_ADVANCED(MOO_ENABLE_PROJECT)


###################################################################
# flags
#

SET(MOO_COMPILE_DEFINITIONS_DEBUG -DMOO_DEBUG_ENABLED)
SET(MOO_COMPILE_DEFINITIONS_RELEASE -DG_DISABLE_CAST_CHECKS -DG_DISABLE_ASSERT)

SET(MOO_ENABLE_UNIT_TESTS OFF CACHE BOOL "Build unit tests")
IF(MOO_ENABLE_UNIT_TESTS)
  ADD_DEFINITIONS(-DMOO_ENABLE_UNIT_TESTS)
ENDIF(MOO_ENABLE_UNIT_TESTS)

IF(MOO_DEBUG)
  ADD_DEFINITIONS(${MOO_COMPILE_DEFINITIONS_DEBUG})
ELSE(MOO_DEBUG)
  ADD_DEFINITIONS(${MOO_COMPILE_DEFINITIONS_RELEASE})
ENDIF(MOO_DEBUG)


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


IF(NOT GIO_FOUND)
  INCLUDE_DIRECTORIES(${MOO_SOURCE_DIR}/moo/mooutils/newgtk/glib-2.16)

  MOO_CHECK_HEADERS(dirent.h float.h limits.h pwd.h grp.h sys/param.h sys/poll.h sys/resource.h)
  MOO_CHECK_HEADERS(sys/time.h sys/times.h sys/wait.h unistd.h values.h)
  MOO_CHECK_HEADERS(sys/select.h sys/types.h stdint.h sched.h malloc.h)
  MOO_CHECK_HEADERS(sys/vfs.h sys/mount.h sys/vmount.h sys/statfs.h sys/statvfs.h)
  MOO_CHECK_HEADERS(mntent.h sys/mnttab.h sys/vfstab.h sys/mntctl.h sys/sysctl.h fstab.h)

  CHECK_STRUCT_HAS_MEMBER("struct stat" st_mtimensec "sys/stat.h" HAVE_STAT_ST_MTIMENSEC)
  CHECK_STRUCT_HAS_MEMBER("struct stat" st_mtim.tv_nsec "sys/stat.h" HAVE_STAT_ST_MTIM_TV_NSEC)
  CHECK_STRUCT_HAS_MEMBER("struct stat" st_atimensec "sys/stat.h" HAVE_STAT_ST_ATIMENSEC)
  CHECK_STRUCT_HAS_MEMBER("struct stat" st_ctimensec "sys/stat.h" HAVE_STAT_ST_CTIMENSEC)
  CHECK_STRUCT_HAS_MEMBER("struct stat" st_ctim.tv_nsec "sys/stat.h" HAVE_STAT_ST_CTIM_TV_NSEC)
  CHECK_STRUCT_HAS_MEMBER("struct stat" st_blksize "sys/stat.h" HAVE_STAT_ST_BLKSIZE)
  CHECK_STRUCT_HAS_MEMBER("struct stat" st_blocks "sys/stat.h" HAVE_STAT_ST_BLOCKS)
  CHECK_STRUCT_HAS_MEMBER("struct statfs" f_fstypename "sys/statfs.h" HAVE_STATFS_F_FSTYPENAME)
  CHECK_STRUCT_HAS_MEMBER("struct statfs" f_bavail "sys/statfs.h" HAVE_STATFS_F_BAVAIL)

  # struct statvfs.f_basetype is available on Solaris but not for Linux.
  CHECK_STRUCT_HAS_MEMBER("struct statvfs" f_basetype sys/statvfs.h HAVE_STATFS_F_BASETYPE)

  # Check for some functions
  MOO_CHECK_FUNCTIONS(lstat strerror strsignal memmove vsnprintf stpcpy strcasecmp strncasecmp poll getcwd vasprintf setenv unsetenv getc_unlocked readlink symlink fdwalk)
  MOO_CHECK_FUNCTIONS(chown lchown fchmod fchown link statvfs statfs utimes getgrgid getpwuid)
  MOO_CHECK_FUNCTIONS(getmntent_r setmntent endmntent hasmntopt getmntinfo)
ENDIF(NOT GIO_FOUND)


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


IF(NOT WIN32)
  SET(MOO_ENABLE_GENERATED_FILES ON CACHE BOOL "run gtk-update-icon-cache on install")
ELSE(NOT WIN32)
  SET(MOO_ENABLE_GENERATED_FILES OFF)
ENDIF(NOT WIN32)

IF(WIN32 AND NOT MSVC)
  FIND_PROGRAM(WINDRES_EXECUTABLE windres
               DOC "Path to the windres executable")
  IF(NOT WINDRES_EXECUTABLE)
    MESSAGE(FATAL_ERROR "Could not find windres")
  ENDIF(NOT WINDRES_EXECUTABLE)
ENDIF(WIN32 AND NOT MSVC)

SET(MOO_BROKEN_GTK_THEME)
MOO_DEFINE_H(MOO_BROKEN_GTK_THEME)

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
