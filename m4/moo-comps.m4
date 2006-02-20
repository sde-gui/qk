##############################################################################
# MOO_COMPONENTS()
#
AC_DEFUN([MOO_COMPONENTS],[
    AC_ARG_WITH([mooapp],
        AC_HELP_STRING([--without-mooapp], [disable building mooapp]),
        [build_mooapp=$withval],
        [build_mooapp=yes]
    )
    AC_ARG_WITH([mooedit],
        AC_HELP_STRING([--without-mooedit], [disable building mooedit]),
        [build_mooedit=$withval],
        [build_mooedit=yes]
    )
    AC_ARG_WITH([mooutils],
        AC_HELP_STRING([--without-mooutils], [disable building mooutils]),
        [build_mooutils=$withval],
        [build_mooutils=yes]
    )
    AC_ARG_WITH([mooterm],
        AC_HELP_STRING([--without-mooterm], [disable building mooterm]),
        [build_mooterm=$withval],
        [build_mooterm=yes]
    )

    if test x$MOO_OS_CYGWIN = "xyes"; then
        build_mooutils="no"
        build_mooedit="no"
        build_mooapp="no"
    fi

    if test "x$build_mooapp" != "xno"; then
        build_mooedit="yes"
    fi
    if test "x$build_mooedit" != "xno"; then
        build_mooutils="yes"
    fi
    if test "x$build_mooterm" != "xno" -a x$MOO_OS_CYGWIN != "xyes"; then
        build_mooutils="yes"
    fi

    AM_CONDITIONAL(MOO_BUILD_UTILS,     test "x$build_mooutils"  != "xno")
    AM_CONDITIONAL(MOO_BUILD_EDIT,      test "x$build_mooedit"   != "xno")
    AM_CONDITIONAL(MOO_BUILD_TERM,      test "x$build_mooterm"   != "xno")
    AM_CONDITIONAL(MOO_BUILD_APP,       test "x$build_mooapp"    != "xno")

    MOO_BUILD_UTILS=0
    MOO_BUILD_EDIT=0
    MOO_BUILD_TERM=0
    MOO_BUILD_APP=0
    MOO_BUILD_COMPS=

    if test "x$build_mooutils" != "xno"; then
        AC_DEFINE(MOO_BUILD_UTILS,, [build mooutils])
        MOO_BUILD_UTILS=1
        MOO_BUILD_COMPS="mooutils $MOO_BUILD_COMPS"
    fi
    if test "x$build_mooedit" != "xno"; then
        AC_DEFINE(MOO_BUILD_EDIT,, [build mooedit])
        MOO_BUILD_EDIT=1
        MOO_BUILD_COMPS="mooedit $MOO_BUILD_COMPS"
    fi
    if test "x$build_mooterm" != "xno"; then
        AC_DEFINE(MOO_BUILD_TERM,, [build mooterm])
        MOO_BUILD_TERM=1
        if test "x$MOO_OS_BSD" = "xyes"; then
            MOO_LIBS="-lutil $MOO_LIBS"
        fi
        MOO_BUILD_COMPS="mooterm $MOO_BUILD_COMPS"
    fi
    if test "x$build_mooapp" != "xno"; then
        AC_DEFINE(MOO_BUILD_APP,, [build mooapp])
        MOO_BUILD_APP=1
        MOO_BUILD_COMPS="mooapp $MOO_BUILD_COMPS"
    fi

    AC_SUBST(MOO_BUILD_UTILS)
    AC_SUBST(MOO_BUILD_EDIT)
    AC_SUBST(MOO_BUILD_TERM)
    AC_SUBST(MOO_BUILD_APP)
])
