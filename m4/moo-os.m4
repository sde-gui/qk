##############################################################################
# MOO_AC_CHECK_OS([])
#
AC_DEFUN([MOO_AC_CHECK_OS],[
    MOO_OS_CYGWIN="no"
    MOO_OS_MINGW="no"
    MOO_OS_DARWIN="no"
    MOO_OS_UNIX="no"
    MOO_OS_BSD="no"

    case $host in
    *-*-mingw32*)
        echo "++++ building for MINGW32 ++++"
        MOO_OS_MINGW="yes"
        ;;
    *-*-cygwin*)
        echo "++++ building for CYGWIN ++++"
        MOO_OS_CYGWIN="yes"
        ;;
    *-*-darwin*)
        echo "++++ building for DARWIN ++++"
        MOO_OS_DARWIN="yes"
        MOO_OS_BSD="yes"
        MOO_OS_UNIX="yes"
        ;;
    *-*-freebsd*)
        echo "++++ building for FREEBSD ++++"
        MOO_OS_BSD="yes"
        MOO_OS_UNIX="yes"
        ;;
    *)
        echo "++++ building for UNIX ++++"
        MOO_OS_UNIX="yes"
        ;;
    esac

    if test x$MOO_OS_DARWIN = xyes; then
        AC_DEFINE(MOO_OS_DARWIN, 1, [darwin])
    fi
    if test x$MOO_OS_BSD = xyes; then
        AC_DEFINE(MOO_OS_BSD, 1, [bsd])
    fi
    if test x$MOO_OS_UNIX = xyes; then
        AC_DEFINE(MOO_OS_UNIX, 1, [unix])
    fi
    if test x$MOO_OS_MINGW = xyes; then
        AC_DEFINE(MOO_OS_MINGW, 1, [mingw])
    fi
    if test x$MOO_OS_CYGWIN = xyes; then
        AC_DEFINE(MOO_OS_CYGWIN, 1, [cygwin])
    fi

    AM_CONDITIONAL(MOO_OS_UNIX, test x$MOO_OS_UNIX = "xyes")
    AM_CONDITIONAL(MOO_OS_MINGW, test x$MOO_OS_MINGW = "xyes")
    AM_CONDITIONAL(MOO_OS_CYGWIN, test x$MOO_OS_CYGWIN = "xyes")
    AM_CONDITIONAL(MOO_OS_DARWIN, test x$MOO_OS_DARWIN = "xyes")
    AM_CONDITIONAL(MOO_OS_BSD, test x$MOO_OS_BSD = "xyes")
])
