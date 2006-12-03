##############################################################################
# MOO_AC_DEBUG()
#
AC_DEFUN([MOO_AC_DEBUG],[

AC_REQUIRE([MOO_PKG_CHECK_GTK_VERSIONS])

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

AC_ARG_ENABLE(all-gcc-warnings,
AC_HELP_STRING([--enable-all-gcc-warnings],[enable most of gcc warnings and turn on -pedantic mode (default = NO)]),[
  if test "x$enable_all_gcc_warnings" = "xno"; then
    all_gcc_warnings="no"
  elif test "x$enable_all_gcc_warnings" = "xfatal"; then
    all_gcc_warnings="yes"
    warnings_fatal="yes"
  else
    all_gcc_warnings="yes"
    warnings_fatal="no"
  fi
],[
  all_gcc_warnings="no"
])

AC_ARG_ENABLE(all-intel-warnings,
AC_HELP_STRING([--enable-all-intel-warnings], [enable most of intel compiler warnings (default = NO)]),[
  if test x$enable_all_intel_warnings = "xno"; then
    all_intel_warnings="no"
  else
    all_intel_warnings="yes"
  fi
],[
  all_intel_warnings="no"
])

if test x$all_intel_warnings = "xyes"; then
    MOO_DEBUG_CFLAGS="$MOO_DEBUG_CFLAGS -Wall -Wcheck -w2 -wd981 -wd188"
elif test x$all_gcc_warnings = "xyes"; then
MOO_DEBUG_CFLAGS="$MOO_DEBUG_GCC_CFLAGS $MOO_DEBUG_CFLAGS -W -Wall -Wpointer-arith dnl
-Wcast-align -Wsign-compare -Winline -Wreturn-type dnl
-Wwrite-strings -Wmissing-prototypes -Wmissing-declarations dnl
-Wmissing-noreturn -Wmissing-format-attribute -Wnested-externs dnl
-Wdisabled-optimization"
MOO_PYTHON_DEBUG_CFLAGS="$MOO_DEBUG_GCC_CFLAGS $MOO_PYTHON_DEBUG_CFLAGS -Wall -Wpointer-arith dnl
-Wcast-align -Wsign-compare -Winline -Wreturn-type dnl
-Wmissing-prototypes -Wmissing-declarations dnl
-Wmissing-noreturn -Wmissing-format-attribute -Wnested-externs dnl
-Wdisabled-optimization"
elif test x$GCC = "xyes"; then
    MOO_DEBUG_CFLAGS=$MOO_DEBUG_GCC_CFLAGS
fi

if test x$MOO_DEBUG = "xyes"; then
moo_debug_flags="-DG_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED dnl
-DGDK_DISABLE_DEPRECATED -DENABLE_DEBUG -DENABLE_PROFILE dnl
-DG_ENABLE_DEBUG -DG_ENABLE_PROFILE -DMOO_DEBUG=1"
else
  moo_debug_flags="-DNDEBUG=1 -DG_DISABLE_CAST_CHECKS -DG_DISABLE_ASSERT"
fi

MOO_DEBUG_CFLAGS="$MOO_DEBUG_CFLAGS $moo_debug_flags"
MOO_PYTHON_DEBUG_CFLAGS="$MOO_PYTHON_DEBUG_CFLAGS $moo_debug_flags"

if test x$MOO_ENABLE_TESTS = "xyes"; then
  MOO_DEBUG_CFLAGS="$MOO_DEBUG_CFLAGS -DMOO_ENABLE_TESTS"
fi

AC_SUBST(MOO_DEBUG_CFLAGS)
AC_SUBST(MOO_PYTHON_DEBUG_CFLAGS)

AC_MSG_CHECKING(for C compiler debug options)
if test "x$MOO_DEBUG_CFLAGS" = "x"; then
  AC_MSG_RESULT(None)
else
  AC_MSG_RESULT($MOO_DEBUG_CFLAGS)
fi
])
