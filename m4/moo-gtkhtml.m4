##############################################################################
# MOO_AC_GTKHTML()
#
AC_DEFUN([MOO_AC_GTKHTML],[
    MOO_USE_GTKHTML=auto

    AC_ARG_WITH([gtkhtml],
        AC_HELP_STRING([--with-gtkhtml], [whether to use gtkhtml (default = AUTO)]),[
        if test x$with_gtkhtml = "xyes"; then
            MOO_USE_GTKHTML="yes"
        else
            MOO_USE_GTKHTML="no"
        fi
    ])

    if test x$MOO_USE_GTKHTML != xno; then
        PKG_CHECK_MODULES(GTKHTML, [libgtkhtml-2.0], [
            MOO_USE_GTKHTML=yes
            MOO_GTKHTML_PACKAGE=libgtkhtml-2.0
            AC_DEFINE(MOO_USE_GTKHTML, 1, [use libgtkhtml-2.0])
        ],[
            if test x$MOO_USE_GTKHTML = xyes; then
                AC_MSG_ERROR([libgtkhtml-2 not found.])
            else
                MOO_USE_GTKHTML=no
            fi
        ])
    fi

    AC_SUBST(MOO_USE_GTKHTML)
    AC_SUBST(MOO_GTKHTML_PACKAGE)
    AM_CONDITIONAL(MOO_USE_GTKHTML, test x$MOO_USE_GTKHTML = "xyes")
])
