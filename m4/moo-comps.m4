##############################################################################
# MOO_COMPONENTS(default-yes,default-no)
#
AC_DEFUN([MOO_COMPONENTS],[
  m4_foreach([comp], [utils, edit, term, app],
             [build_moo[]comp=yes])
  m4_foreach([comp], $2,
             [build_moo[]comp=no])

  m4_foreach([comp], [$1],
  [AC_ARG_WITH([moo[]comp],
    AC_HELP_STRING([--with-moo[]comp], [enable moo[]comp component (default = YES)]),
    [build_moo[]comp=$withval])])
  m4_foreach([comp], [$2],
  [AC_ARG_WITH([moo[]comp],
    AC_HELP_STRING([--with-moo[]comp], [enable moo[]comp component (default = NO)]),
    [build_moo[]comp=$withval])])

  if test "x$MOO_OS_CYGWIN" = "xyes"; then
    build_mooutils="no"
    build_mooedit="no"
    build_mooapp="no"
    build_mooterm="yes"
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

  MOO_BUILD_COMPS=

  m4_foreach([comp], [utils, edit, term, app],[
    AM_CONDITIONAL(MOO_BUILD_[]m4_toupper(comp), test "x$build_moo[]comp" != "xno")
    MOO_BUILD_[]m4_toupper(comp)=0
    if test "x$build_moo[]comp" != "xno"; then
      AC_DEFINE(MOO_BUILD_[]m4_toupper(comp), [1], [build moo]comp)
      MOO_BUILD_[]m4_toupper(comp)=1
      MOO_BUILD_COMPS="moo[]comp $MOO_BUILD_COMPS"
    fi
    AC_SUBST(MOO_BUILD_[]m4_toupper(comp))
  ])

  if test "x$build_mooterm" != "xno" -a "x$MOO_OS_BSD" = "xyes"; then
    MOO_LIBS="-lutil $MOO_LIBS"
  fi
])
