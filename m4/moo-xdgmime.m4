##############################################################################
# MOO_AC_XDGMIME([])
# does nothing, just defines MOO_USE_XDGMIME on unix
#
AC_DEFUN_ONCE([MOO_AC_XDGMIME],[
    MOO_USE_XDGMIME=true
    AC_DEFINE(MOO_USE_XDGMIME, 1, [use xdgmime])
    AM_CONDITIONAL(MOO_USE_XDGMIME, true)
])
