##############################################################################
# _MOO_AC_CHECK_PYGTK(action-if-found,action-if-not-found)
# checks pygtk stuff
#
AC_DEFUN([_MOO_AC_CHECK_PYGTK],[
  AC_REQUIRE([MOO_AC_CHECK_OS])

  PKG_CHECK_MODULES(PYGTK,pygtk-2.0 >= 2.6.0,[
    AC_MSG_CHECKING([whether pygtk can be used])
    save_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS $PYGTK_CFLAGS $PYTHON_INCLUDES"
    save_CFLAGS="$CFLAGS"
    CFLAGS="$CFLAGS $PYGTK_CFLAGS $PYTHON_INCLUDES"
    save_LIBS="$LIBS"
    LIBS="$LIBS $PYGTK_LIBS $PYTHON_LIBS $PYTHON_EXTRA_LIBS"
    save_LDFLAGS="$LDFLAGS"
    LDFLAGS="$LDFLAGS $PYGTK_LDFLAGS $PYTHON_LDFLAGS $PYTHON_EXTRA_LDFLAGS"

    AC_LINK_IFELSE([AC_LANG_SOURCE([[
    #include <pygobject.h>
    int main ()
    {
        PyObject *object = pygobject_new (NULL);
        return 0;
    }]])],[
        AC_MSG_RESULT(yes)
        PYGTK_DEFS_DIR=`$PKG_CONFIG --variable=defsdir pygtk-2.0`
        AC_SUBST(PYGTK_DEFS_DIR)
        PYGTK_CODEGEN_DIR=`$PKG_CONFIG --variable=codegendir pygtk-2.0`
        AC_SUBST(PYGTK_CODEGEN_DIR)
        AC_MSG_NOTICE([pygtk defs dir: $PYGTK_DEFS_DIR])
        m4_if([$1],[],[:],[$1])
    ],[
        AC_MSG_RESULT([no])
        m4_if([$2],[],[:],[$2])
    ])

    CFLAGS="$save_CFLAGS"
    CPPFLAGS="$save_CPPFLAGS"
    LIBS="$save_LIBS"
    LDFLAGS="$save_LDFLAGS"
  ],[
    m4_if([$2],[],[:],[$2])
  ])
])


##############################################################################
# MOO_AC_PYTHON()
#
AC_DEFUN_ONCE([MOO_AC_PYTHON],[
  AC_REQUIRE([MOO_AC_CHECK_OS])

  MOO_ENABLE_PYTHON=true
  _moo_want_python="auto"
  _moo_python_version=2.2

  AC_ARG_WITH([python],AC_HELP_STRING([--with-python], [whether to compile python support (default = YES)]),[
    if test "x$with_python" = "xno"; then
      MOO_ENABLE_PYTHON=false
    elif test "x$with_python" = "xyes"; then
      _moo_want_python="yes"
      _moo_python_version="2.2"
    else
      _moo_want_python="yes"
      _moo_python_version="$with_python"
    fi
  ])

  if test "x$MOO_OS_CYGWIN" = "xyes"; then
    MOO_ENABLE_PYTHON=false
  fi

  if $MOO_ENABLE_PYTHON; then
    MOO_ENABLE_PYTHON=false
    MOO_AC_CHECK_PYTHON($_moo_python_version,[
      _MOO_AC_CHECK_PYGTK([
        MOO_ENABLE_PYTHON=true
        _MOO_SPLIT_VERSION_PKG(PYGTK, pygtk-2.0)
        AC_SUBST(PYGTK_VERSION)
        AC_SUBST(PYGTK_MAJOR_VERSION)
        AC_SUBST(PYGTK_MINOR_VERSION)
        AC_SUBST(PYGTK_MICRO_VERSION)
      ])
    ])

    if $MOO_ENABLE_PYTHON; then
      AC_MSG_NOTICE([compiling python support])
    elif test "x$_moo_want_python" = "xyes"; then
      AC_MSG_ERROR([python support requested but python cannot be used])
    elif test "x$_moo_want_python" = "xauto"; then
      AC_MSG_WARN([disabled python support])
    else
      AC_MSG_NOTICE([disabled python support])
    fi
  fi

  AM_CONDITIONAL(MOO_ENABLE_PYTHON, $MOO_ENABLE_PYTHON)
  if $MOO_ENABLE_PYTHON; then
    AC_DEFINE(MOO_ENABLE_PYTHON, 1, [build python bindings and plugin])
  fi

  if $MOO_ENABLE_PYTHON; then
    MOO_CFLAGS="$MOO_CFLAGS $PYGTK_CFLAGS $PYTHON_INCLUDES"
    MOO_CXXFLAGS="$MOO_CXXFLAGS $PYGTK_CFLAGS $PYTHON_INCLUDES"
    MOO_LIBS="$MOO_LIBS $PYGTK_LIBS $PYTHON_LIBS $PYTHON_EXTRA_LIBS $PYGTK_LDFLAGS $PYTHON_LDFLAGS $PYTHON_EXTRA_LDFLAGS"
  fi
])
