##############################################################################
# MOO_AC_DEBUG()
#
AC_DEFUN([MOO_AC_DEBUG],[
    MOO_DEBUG="no"
    MOO_DEBUG_CFLAGS=
    MOO_DEBUG_CXXFLAGS=

    AC_ARG_ENABLE(debug,
    AC_HELP_STRING([--enable-debug],[enable debug options (default = NO)]),[
        if test "x$enable_debug" = "xno"; then
            MOO_DEBUG="no"
        else
            MOO_DEBUG="yes"
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
        MOO_DEBUG_CFLAGS="$MOO_DEBUG_CFLAGS -Wall -Wcheck -w2"
        MOO_DEBUG_CXXFLAGS="$MOO_DEBUG_CXXFLAGS -Wall -Wcheck -w2 -wd981,279,858,1418 -wd383"
    elif test x$all_gcc_warnings = "xyes"; then
MOO_DEBUG_CFLAGS="$MOO_DEBUG_GCC_CFLAGS $MOO_DEBUG_CFLAGS -W -Wall -Wpointer-arith dnl
-Wcast-align -Wsign-compare -Winline -Wreturn-type dnl
-Wwrite-strings -Wmissing-prototypes -Wmissing-declarations dnl
-Wmissing-noreturn -Wmissing-format-attribute -Wnested-externs dnl
-Wdisabled-optimization" -Wdeclaration-after-statement #-Wunreachable-code
MOO_PYTHON_DEBUG_CFLAGS="$MOO_DEBUG_GCC_CFLAGS $MOO_PYTHON_DEBUG_CFLAGS -Wall -Wpointer-arith dnl
-Wcast-align -Wsign-compare -Winline -Wreturn-type dnl
-Wmissing-prototypes -Wmissing-declarations dnl
-Wmissing-noreturn -Wmissing-format-attribute -Wnested-externs dnl
-Wdisabled-optimization" #-Wunreachable-code
MOO_DEBUG_CXXFLAGS="$MOO_DEBUG_GCC_CXXFLAGS $MOO_DEBUG_CXXFLAGS -W -Wall -Woverloaded-virtual dnl
-Wpointer-arith -Wcast-align -Wsign-compare -Wnon-virtual-dtor dnl
-Wno-long-long -Wundef -Wconversion -Wchar-subscripts -Wwrite-strings dnl
-Wmissing-format-attribute -Wcast-align -Wdisabled-optimization dnl
-Wnon-template-friend -Wsign-promo -Wno-ctor-dtor-privacy"
    elif test x$GCC = "xyes"; then
        MOO_DEBUG_CFLAGS=$MOO_DEBUG_GCC_CFLAGS
        MOO_DEBUG_CXXFLAGS=$MOO_DEBUG_GCC_CXXFLAGS
    fi

    if test x$MOO_DEBUG = "xyes"; then
        moo_debug_flags="-DG_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED dnl
 -DGDK_DISABLE_DEPRECATED -DDEBUG -DENABLE_DEBUG -DENABLE_PROFILE dnl
 -DG_ENABLE_DEBUG -DG_ENABLE_PROFILE -DMOO_DEBUG=1"
    else
        moo_debug_flags="-DNDEBUG=1 -DG_DISABLE_CAST_CHECKS -DG_DISABLE_ASSERT"
    fi

    MOO_DEBUG_CFLAGS="$MOO_DEBUG_CFLAGS $moo_debug_flags"
    MOO_PYTHON_DEBUG_CFLAGS="$MOO_PYTHON_DEBUG_CFLAGS $moo_debug_flags"
    MOO_DEBUG_CXXFLAGS="$MOO_DEBUG_CXXFLAGS $moo_debug_flags"

    if test "x$all_gcc_warnings" = "xyes" -a "x$warnings_fatal" = "xyes"; then
        MOO_DEBUG_CXXFlAGS_NO_WERROR=$MOO_DEBUG_CXXFLAGS
        MOO_DEBUG_CXXFLAGS="-Werror $MOO_DEBUG_CXXFLAGS"
    fi

    AC_SUBST(MOO_DEBUG_CFLAGS)
    AC_SUBST(MOO_PYTHON_DEBUG_CFLAGS)
    AC_SUBST(MOO_DEBUG_CXXFLAGS)
    AC_SUBST(MOO_DEBUG_CXXFlAGS_NO_WERROR)

    AC_MSG_CHECKING(for C compiler debug options)
    if test "x$MOO_DEBUG_CFLAGS" = "x"; then
        AC_MSG_RESULT(None)
    else
        AC_MSG_RESULT($MOO_DEBUG_CFLAGS)
    fi
    AC_MSG_CHECKING(for C++ compiler debug options)
    if test "x$MOO_DEBUG_CXXFLAGS" = "x"; then
        AC_MSG_RESULT(None)
    else
        AC_MSG_RESULT($MOO_DEBUG_CXXFLAGS)
    fi
])
