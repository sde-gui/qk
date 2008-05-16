AC_DEFUN([_MOO_OPTION],[
  AC_ARG_ENABLE([$1],AC_HELP_STRING([$2],[$3]),[
    if test "x$enableval" = "xyes"; then
      $4=true
    else
      $4=false
    fi
  ])
])

dnl must be called after MOO_AC_PYTHON
AC_DEFUN_ONCE([MOO_AC_LIB],[
  if test "x$MOO_OS_UNIX" = "xyes"; then
    MOO_INSTALL_LIB=false
    MOO_BUILD_MOO_MODULE=false
    MOO_BUILD_PYTHON_PLUGIN=false
    MOO_PYTHON_BUILTIN=$MOO_USE_PYTHON
  else
    MOO_INSTALL_LIB=true
    MOO_BUILD_MOO_MODULE=true
    MOO_BUILD_PYTHON_PLUGIN=true
    MOO_PYTHON_BUILTIN=false
  fi

  if test "x$MOO_BUILD_MEDIT" != "xfalse"; then
    MOO_BUILD_MEDIT=true
  fi

  _MOO_OPTION([libmoo],[--enable-libmoo],[build and install libmoo library (default = NO)],[MOO_INSTALL_LIB])
  _MOO_OPTION([moo-module],[--enable-moo-module], [build and install python extension module (default = NO)],[MOO_BUILD_MOO_MODULE])
  _MOO_OPTION([medit],[--disable-medit], [do not build medit (default = NO)],[MOO_BUILD_MEDIT])

  if $MOO_USE_PYTHON; then
    if $MOO_INSTALL_LIB; then
      MOO_BUILD_PYTHON_PLUGIN=true
      MOO_BUILD_MOO_MODULE=true
      MOO_PYTHON_BUILTIN=false
    elif $MOO_BUILD_MOO_MODULE; then
      MOO_BUILD_PYTHON_PLUGIN=false
      MOO_PYTHON_BUILTIN=true
    else
      MOO_BUILD_PYTHON_PLUGIN=false
      MOO_PYTHON_BUILTIN=true
    fi
  else
    MOO_BUILD_MOO_MODULE=false
    MOO_BUILD_PYTHON_PLUGIN=false
    MOO_PYTHON_BUILTIN=false
  fi

  AM_CONDITIONAL(MOO_INSTALL_LIB, $MOO_INSTALL_LIB)
  AM_CONDITIONAL(MOO_BUILD_PYTHON_PLUGIN, $MOO_BUILD_PYTHON_PLUGIN)
  AM_CONDITIONAL(MOO_BUILD_MOO_MODULE, $MOO_BUILD_MOO_MODULE)
  AM_CONDITIONAL(MOO_BUILD_MEDIT, $MOO_BUILD_MEDIT)
  AM_CONDITIONAL(MOO_PYTHON_BUILTIN, $MOO_PYTHON_BUILTIN)

  if $MOO_PYTHON_BUILTIN; then
    AC_DEFINE(MOO_PYTHON_BUILTIN, 1, [MOO_PYTHON_BUILTIN])
  fi
])
