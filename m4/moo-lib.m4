AC_DEFUN([_MOO_AC_LIB],[
  AC_ARG_ENABLE(libmoo,
    AC_HELP_STRING([--enable-libmoo], [build and install libmoo library (default = NO)]),
    [MOO_INSTALL_LIB="$enableval"],[
      if test x$MOO_OS_UNIX != xyes; then
        MOO_INSTALL_LIB="yes"
      else
        MOO_INSTALL_LIB="no"
      fi
    ])

  AC_ARG_ENABLE(libmoo-headers,
    AC_HELP_STRING([--enable-libmoo-headers], [install libmoo library headers (default = NO)]),
    [MOO_INSTALL_HEADERS="$enableval"],[MOO_INSTALL_HEADERS="no"]
  )

  AC_ARG_ENABLE(python-plugin,
    AC_HELP_STRING([--enable-python-plugin], [build python plugin as a separate library (default = NO)]),
    [MOO_BUILD_PYTHON_PLUGIN="$enableval"],[
      if test "x$MOO_OS_UNIX" != "xyes"; then
        MOO_BUILD_PYTHON_PLUGIN="yes"
      else
        MOO_BUILD_PYTHON_PLUGIN="no"
      fi
    ])

  AC_ARG_ENABLE(moo-module,
    AC_HELP_STRING([--enable-moo-module], [install python extension module (default = NO)]),
    [MOO_BUILD_MOO_MODULE="$enableval"],[
      if test "x$MOO_OS_UNIX" != "xyes"; then
        MOO_BUILD_MOO_MODULE="yes"
      else
        MOO_BUILD_MOO_MODULE="no"
      fi
    ])

  AC_ARG_ENABLE(medit,
    AC_HELP_STRING([--disable-medit], [do not build medit (default = NO)]),
    [MOO_BUILD_MEDIT="$enableval"],[MOO_BUILD_MEDIT="yes"])

  AM_CONDITIONAL(MOO_INSTALL_LIB, test "x$MOO_INSTALL_LIB" = "xyes")
  AM_CONDITIONAL(MOO_INSTALL_HEADERS, test "x$MOO_INSTALL_HEADERS" = "xyes")
  AM_CONDITIONAL(MOO_BUILD_PYTHON_PLUGIN, test "x$MOO_BUILD_PYTHON_PLUGIN" = "xyes")
  AM_CONDITIONAL(MOO_BUILD_MOO_MODULE, test "x$MOO_BUILD_MOO_MODULE" = "xyes")
  AM_CONDITIONAL(MOO_BUILD_MEDIT, test "x$MOO_BUILD_MEDIT" = "xyes")

  if test "x$MOO_BUILD_PYTHON_PLUGIN" = "xyes"; then
    AC_DEFINE(MOO_BUILD_PYTHON_PLUGIN, 1, [build python plugin])
  fi

  if test "x$MOO_BUILD_MOO_MODULE" = "xyes"; then
    AC_DEFINE(MOO_BUILD_MOO_MODULE, 1, [build python extension module])
  fi
])
