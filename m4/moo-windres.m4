##############################################################################
# MOO_AC_PROG_WINDRES
# checks whether windres is available, pretty broken
#
AC_DEFUN([MOO_AC_PROG_WINDRES],[
case $host in
*-*-mingw*|*-*-cygwin*)
    AC_MSG_CHECKING([for windres])
    found=
    if test -z $WINDRES; then
        if test "x$CC" != "x"; then
            WINDRES=`echo $CC | sed "s/gcc$//"`windres
        else
            WINDRES=windres
        fi
    fi
    found=`$WINDRES --version 2>/dev/null`
    if test "x$found" != "x"; then
        AC_MSG_RESULT([found $WINDRES])
        AC_SUBST(WINDRES)
    else
        AC_MSG_RESULT([not found])
    fi
    ;;
*)
    ;;
esac
])
