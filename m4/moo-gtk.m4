##############################################################################
# _MOO_CHECK_VERSION(pkg-name)
#
AC_DEFUN([_MOO_CHECK_VERSION],[
    if test x$MOO_OS_CYGWIN != xyes; then
        PKG_CHECK_MODULES($1,$2)

        AC_MSG_CHECKING($1 version)
        $1[]_VERSION=`$PKG_CONFIG --modversion $2`

        i=0
        for part in `echo $[]$1[]_VERSION | sed 's/\./ /g'`; do
            i=`expr $i + 1`
            eval part$i=$part
        done

        $1[]_MAJOR_VERSION=$part1
        $1[]_MINOR_VERSION=$part2
        $1[]_MICRO_VERSION=$part3

        for i in 2 4 6 8 10 12 14; do
            if test $[]$1[]_MINOR_VERSION -ge $i; then
                eval moo_ver_2_[]$i=yes
            else
                eval moo_ver_2_[]$i=no
            fi
        done

        AC_MSG_RESULT($[]$1[]_MAJOR_VERSION.$[]$1[]_MINOR_VERSION.$[]$1[]_MICRO_VERSION)
    fi

    AM_CONDITIONAL($1[]_2_14, test x$moo_ver_2_14 = "xyes")
    AM_CONDITIONAL($1[]_2_12, test x$moo_ver_2_12 = "xyes")
    AM_CONDITIONAL($1[]_2_10, test x$moo_ver_2_10 = "xyes")
    AM_CONDITIONAL($1[]_2_8, test x$moo_ver_2_8 = "xyes")
    AM_CONDITIONAL($1[]_2_6, test x$moo_ver_2_6 = "xyes")
    AM_CONDITIONAL($1[]_2_4, test x$moo_ver_2_4 = "xyes")
    AM_CONDITIONAL($1[]_2_2, test x$moo_ver_2_2 = "xyes")
])


##############################################################################
# MOO_PKG_CHECK_GTK_VERSIONS
#
AC_DEFUN([MOO_PKG_CHECK_GTK_VERSIONS],[
    AC_REQUIRE([MOO_AC_CHECK_OS])
    _MOO_CHECK_VERSION(GTK, gtk+-2.0)
    _MOO_CHECK_VERSION(GLIB, glib-2.0)
    _MOO_CHECK_VERSION(GDK, gdk-2.0)

    AC_ARG_ENABLE([printing], AC_HELP_STRING([--enable-printing], [whether to enable printing support with gtk >= 2.9 (default = NO, it is UNSTABLE)]), [
            if test x$enable_printing = "xyes"; then
                MOO_ENABLE_PRINTING="yes"
            else
                MOO_ENABLE_PRINTING="no"
            fi
        ],[
            MOO_ENABLE_PRINTING="no"
    ])

    if test $GTK_MINOR_VERSION -lt 9; then
        AC_MSG_NOTICE([GTK version is $GTK_VERSION, no printing])
        MOO_ENABLE_PRINTING="no"
    elif test x$MOO_ENABLE_PRINTING = xno; then
        AC_MSG_NOTICE([printing disabled])
    else
        AC_DEFINE(MOO_ENABLE_PRINTING, 1, [enable printing])
        AC_MSG_NOTICE([printing enabled])
    fi

    AM_CONDITIONAL(MOO_ENABLE_PRINTING, test x$MOO_ENABLE_PRINTING = xyes)
])
