##############################################################################
# MOO_AC_XDGMIME([])
# does nothing, just defines MOO_USE_XDGMIME on unix
#
AC_DEFUN([MOO_AC_XDGMIME],[
    AC_REQUIRE([MOO_AC_CHECK_OS])

    if test x$MOO_OS_UNIX = "xyes"; then
        AC_DEFINE(MOO_USE_XDGMIME, 1, [use xdgmime])
    fi
    AM_CONDITIONAL(MOO_USE_XDGMIME, test x$MOO_OS_UNIX = "xyes")
])
