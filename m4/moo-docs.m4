AC_DEFUN([MOO_DOCS],[
  AC_REQUIRE([MOO_AC_SET_DIRS])

  AC_ARG_ENABLE([help],
    AC_HELP_STRING(--enable-help, [build html help files (default = NO). Will not work on all systems]),
    [:],[enable_help=no])
  AM_CONDITIONAL(MOO_ENABLE_HELP, test "x$enable_help" = xyes)
  if test "x$enable_help" = xyes; then
    AC_DEFINE(MOO_ENABLE_HELP, [1], [enable help functionality])
  fi

  AC_ARG_WITH([helpdir],
    AC_HELP_STRING([--with-helpdir=path], [Location for html help files]),
    [if test "$with_helpdir" = yes -o "$with_helpdir" = no; then
       AC_MSG_ERROR("--with-helpdir value must be a path")
     fi
     helpdir="$with_helpdir"
    ],[helpdir="${MOO_DATA_DIR}/help"])

  AC_SUBST(helpdir)
])
