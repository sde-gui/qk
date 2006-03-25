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
            fi
        done

        #if test $[]$1[]_MINOR_VERSION -ge 9; then
        #    eval moo_ver_2_10=yes
        #fi

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
])
