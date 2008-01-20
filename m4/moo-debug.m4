# _MOO_AC_CHECK_COMPILER_OPTIONS(var,options)
AC_DEFUN([_MOO_AC_CHECK_COMPILER_OPTIONS],[
  AC_LANG_SAVE
  AC_LANG_C
  for opt in $2; do
    AC_MSG_CHECKING(whether C compiler accepts $opt)
    save_CFLAGS="$CFLAGS"
    CFLAGS="$CFLAGS $opt"
    AC_TRY_COMPILE([],[],[$1="$[]$1 $opt"; AC_MSG_RESULT(yes)],[AC_MSG_RESULT(no)])
    CFLAGS="$save_CFLAGS"
  done
  AC_LANG_RESTORE
])

AC_DEFUN([MOO_COMPILER],[
# icc pretends to be gcc or configure thinks it's gcc, but icc doesn't
# error on unknown options, so just don't try gcc options with icc
MOO_ICC=false
MOO_GCC=false
if test "$CC" = "icc"; then
  MOO_ICC=true
elif test "x$GCC" = "xyes"; then
  MOO_GCC=true
fi
])

##############################################################################
# MOO_AC_DEBUG()
#
AC_DEFUN_ONCE([MOO_AC_DEBUG],[

MOO_DEBUG_ENABLED="no"
MOO_DEBUG_CFLAGS=

AC_ARG_ENABLE(tests,
  AC_HELP_STRING([--enable-tests],[build test programs (default = NO)]),[
  if test "x$enable_tests" = "xno"; then
    MOO_ENABLE_TESTS="no"
  else
    MOO_ENABLE_TESTS="yes"
  fi
  ],[
  MOO_ENABLE_TESTS="no"
])
AM_CONDITIONAL(MOO_ENABLE_TESTS, test x$MOO_ENABLE_TESTS = "xyes")

AC_ARG_ENABLE(unit-tests,
  AC_HELP_STRING([--enable-unit-tests],[build unit tests (default = NO)]),[
  if test "x$enable_unit_tests" = "xno"; then
    MOO_ENABLE_UNIT_TESTS="no"
  else
    MOO_ENABLE_UNIT_TESTS="yes"
  fi
  ],[
  MOO_ENABLE_UNIT_TESTS="no"
])
if test x$MOO_ENABLE_UNIT_TESTS = "xyes"; then
  AC_CHECK_LIB(cunit, CU_assertImplementation,[:],[
    AC_MSG_ERROR("CUnit is required for unit tests")
  ])
fi
AM_CONDITIONAL(MOO_ENABLE_UNIT_TESTS, test x$MOO_ENABLE_UNIT_TESTS = "xyes")

AC_ARG_ENABLE(debug,
  AC_HELP_STRING([--enable-debug],[enable debug options (default = NO)]),[
  if test "x$enable_debug" = "xno"; then
    MOO_DEBUG_ENABLED="no"
  else
    MOO_DEBUG_ENABLED="yes"
  fi
  ],[
  MOO_DEBUG_ENABLED="no"
])
AM_CONDITIONAL(MOO_DEBUG_ENABLED, test x$MOO_DEBUG_ENABLED = "xyes")

_moo_all_warnings="no"
AC_ARG_ENABLE(all-warnings,
AC_HELP_STRING([--enable-all-warnings],[enable lot of compiler warnings (default = NO)]),[
  _moo_all_warnings="$enableval"
])
AM_CONDITIONAL(MOO_ALL_WARNINGS, test x$_moo_all_warnings = "xyes")

MOO_COMPILER

MOO_DEBUG_CFLAGS=
if test "x$_moo_all_warnings" = "xyes"; then
  if $MOO_ICC; then
    _MOO_AC_CHECK_COMPILER_OPTIONS(MOO_DEBUG_CFLAGS, [-Wall -Wcheck -w2 -wd981 -wd188 -wd869 -wd556 -wd810])
  elif $MOO_GCC; then
    _MOO_AC_CHECK_COMPILER_OPTIONS(MOO_DEBUG_CFLAGS,
[-W -Wall -Wpointer-arith -Wcast-align -Wsign-compare -Winline -Wreturn-type dnl
-Wwrite-strings -Wmissing-prototypes -Wmissing-declarations dnl
-Wmissing-noreturn -Wmissing-format-attribute -Wnested-externs dnl
-Wdisabled-optimization -Wendif-labels -Wstrict-prototypes dnl
-Wno-missing-field-initializers -Wno-format-y2k])
  fi
else
  if $MOO_GCC; then
    _MOO_AC_CHECK_COMPILER_OPTIONS(MOO_DEBUG_CFLAGS, [-Wall -W -Wno-format-y2k -Wno-missing-field-initializers])
  fi
fi

m4_foreach([wname],[unused, sign-compare, write-strings],[dnl
m4_define([_moo_WNAME],[MOO_W_NO_[]m4_bpatsubst(m4_toupper(wname),-,_)])
_moo_WNAME=
if $MOO_GCC; then
  _MOO_AC_CHECK_COMPILER_OPTIONS(_moo_WNAME,[-Wno-wname])
fi
AC_SUBST(_moo_WNAME)
m4_undefine([_moo_WNAME])
])

if $MOO_GCC; then
  if test x$MOO_DEBUG_ENABLED = "xyes"; then
    _MOO_AC_CHECK_COMPILER_OPTIONS(MOO_DEBUG_CFLAGS,[-fstrict-aliasing -Wstrict-aliasing])
  else
    _MOO_AC_CHECK_COMPILER_OPTIONS(MOO_DEBUG_CFLAGS,[-fno-strict-aliasing])
  fi
fi

if test "x$MOO_DEBUG_ENABLED" = "xyes"; then
_moo_debug_flags="-DG_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED dnl
-DGDK_DISABLE_DEPRECATED -DENABLE_DEBUG -DENABLE_PROFILE dnl
-DG_ENABLE_DEBUG -DG_ENABLE_PROFILE -DMOO_DEBUG_ENABLED=1"
else
_moo_debug_flags="-DNDEBUG=1 -DG_DISABLE_CAST_CHECKS -DG_DISABLE_ASSERT"
fi

MOO_DEBUG_CFLAGS="$MOO_DEBUG_CFLAGS $_moo_debug_flags"

if test "x$MOO_ENABLE_TESTS" = "xyes"; then
  MOO_DEBUG_CFLAGS="$MOO_DEBUG_CFLAGS -DMOO_ENABLE_TESTS"
fi
if test "x$MOO_ENABLE_UNIT_TESTS" = "xyes"; then
  MOO_DEBUG_CFLAGS="$MOO_DEBUG_CFLAGS -DMOO_ENABLE_UNIT_TESTS"
fi

AC_SUBST(MOO_DEBUG_CFLAGS)

AC_MSG_CHECKING(for C compiler debug options)
if test "x$MOO_DEBUG_CFLAGS" = "x"; then
  AC_MSG_RESULT(None)
else
  AC_MSG_RESULT($MOO_DEBUG_CFLAGS)
fi
])
