##############################################################################
# MOO_AC_MODULE()
#
AC_DEFUN([MOO_AC_MODULE],[
    AC_REQUIRE([MOO_AC_PYGTK])

    AC_ARG_ENABLE([moo-module],
    AC_HELP_STRING([--enable-moo-module], [create standalone python module 'moo' (default = YES)]),
    [
        if test x$enable_moo_module = "xno"; then
            build_pymoo="no"
        else
            build_pymoo="yes"
        fi
    ], [
        build_pymoo="yes"
    ])

    if test x$MOO_USE_PYGTK = "xyes"; then
        if test x$build_pymoo != "xno"; then
            build_pymoo="yes"
        fi
    else
        build_pymoo="no"
    fi

    AM_CONDITIONAL(BUILD_PYMOO, test x$build_pymoo = "xyes")
])
