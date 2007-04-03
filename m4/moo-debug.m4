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

##############################################################################
# MOO_AC_DEBUG()
#
AC_DEFUN_ONCE([MOO_AC_DEBUG],[

MOO_DEBUG="no"
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

AC_ARG_ENABLE(debug,
  AC_HELP_STRING([--enable-debug],[enable debug options (default = NO)]),[
  if test "x$enable_debug" = "xno"; then
    MOO_DEBUG="no"
  else
    MOO_DEBUG="yes"
  fi
  ],[
  MOO_DEBUG="no"
])

_moo_all_warnings="no"
AC_ARG_ENABLE(all-warnings,
AC_HELP_STRING([--enable-all-warnings],[enable lot of compiler warnings (default = NO)]),[
  _moo_all_warnings="$withval"
])

# icc pretends to be gcc or configure thinks it's gcc, but icc doesn't
# error on unknown options, so just don't try gcc options with icc
_MOO_ICC=false
_MOO_GCC=false
if test "$CC" = "icc"; then
  _MOO_ICC=true
elif test "x$GCC" = "xyes"; then
  _MOO_GCC=true
fi

MOO_DEBUG_CFLAGS=
if test "x$_moo_all_warnings" = "xyes"; then
  if $_MOO_ICC; then
    _MOO_AC_CHECK_COMPILER_OPTIONS(MOO_DEBUG_CFLAGS, [-Wall -Wcheck -w2 -wd981 -wd188 -wd869 -wd556 -wd810])
  elif $_MOO_GCC; then
    _MOO_AC_CHECK_COMPILER_OPTIONS(MOO_DEBUG_CFLAGS,
[-W -Wall -Wpointer-arith -Wcast-align -Wsign-compare -Winline -Wreturn-type dnl
-Wwrite-strings -Wmissing-prototypes -Wmissing-declarations dnl
-Wmissing-noreturn -Wmissing-format-attribute -Wnested-externs dnl
-Wdisabled-optimization -Wendif-labels -Wstrict-prototypes])
  fi
else
  if $_MOO_GCC; then
    _MOO_AC_CHECK_COMPILER_OPTIONS(MOO_DEBUG_CFLAGS, [-Wall -W])
  fi
fi

m4_foreach([wname],[missing-field-initializers, unused, sign-compare],[dnl
m4_define([_moo_WNAME],[MOO_W_NO_[]m4_bpatsubst(m4_toupper(wname),-,_)])
_moo_WNAME=
if $_MOO_GCC; then
  _MOO_AC_CHECK_COMPILER_OPTIONS(_moo_WNAME,[-Wno-wname])
fi
AC_SUBST(_moo_WNAME)
m4_undefine([_moo_WNAME])
])

if $_MOO_GCC; then
  if test "x$_moo_all_warnings" = "xyes"; then
    _MOO_AC_CHECK_COMPILER_OPTIONS(MOO_CFLAGS,[-fstrict-aliasing])
  else
    _MOO_AC_CHECK_COMPILER_OPTIONS(MOO_CFLAGS,[-fno-strict-aliasing])
  fi
fi

if test "x$MOO_DEBUG" = "xyes"; then
_moo_debug_flags="-DG_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED dnl
-DGDK_DISABLE_DEPRECATED -DENABLE_DEBUG -DENABLE_PROFILE dnl
-DG_ENABLE_DEBUG -DG_ENABLE_PROFILE -DMOO_DEBUG=1"
else
_moo_debug_flags="-DNDEBUG=1 -DG_DISABLE_CAST_CHECKS -DG_DISABLE_ASSERT"
fi

MOO_DEBUG_CFLAGS="$MOO_DEBUG_CFLAGS $_moo_debug_flags"

if test "x$MOO_ENABLE_TESTS" = "xyes"; then
  MOO_DEBUG_CFLAGS="$MOO_DEBUG_CFLAGS -DMOO_ENABLE_TESTS"
fi

AC_SUBST(MOO_DEBUG_CFLAGS)

AC_MSG_CHECKING(for C compiler debug options)
if test "x$MOO_DEBUG_CFLAGS" = "x"; then
  AC_MSG_RESULT(None)
else
  AC_MSG_RESULT($MOO_DEBUG_CFLAGS)
fi
])
