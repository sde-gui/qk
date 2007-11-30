dnl ------------------------------------------------------------------------
dnl MOO_AM_PYTHON_DEVEL_CROSS_MINGW([action-if-found[,action-if-not-found]])
dnl
AC_DEFUN([MOO_AM_PYTHON_DEVEL_CROSS_MINGW],[
  AC_REQUIRE([LT_AC_PROG_SED]) dnl to get $SED set

  AM_PATH_PYTHON

  if test "x$WIN32_PYTHON_HOME" = x; then
    AC_MSG_ERROR([WIN32_PYTHON_HOME environment variable must be set dnl
		  when cross-compiling with mingw])
  fi

  AC_MSG_CHECKING(host system python version)
  if test "x$WIN32_PYTHON_VERSION" = x; then
    # guess python version, very clever heuristics here
    for _ac_python_minor in 3 4 5 6 7 8 9; do
      if test -f "$WIN32_PYTHON_HOME/libs/libpython2$_ac_python_minor.a" -o \
      	-f "$WIN32_PYTHON_HOME/libs/python2$_ac_python_minor.lib" ;
      then
        _ac_pyversion="2.$_ac_python_minor"
        break
      fi
    done
  else
    _ac_pyversion=$WIN32_PYTHON_VERSION
  fi
  if test "x$_ac_pyversion" = x; then
    AC_MSG_ERROR([Could not determine Python version])
  fi
  AC_MSG_RESULT([$_ac_pyversion])
  _ac_pyversion_no_dot=`echo $_ac_pyversion | $SED 's/^2\.*\([[3-9]]\).*/2\1/'`

  AC_MSG_CHECKING(installation directory for python modules)
  if test "x$WIN32_PYTHON_PKG_DIR" != x; then
    _ac_pythondir="$WIN32_PYTHON_PKG_DIR"
  else
    _ac_pythondir="$WIN32_PYTHON_HOME/Lib/site-packages"
  fi
  AC_MSG_RESULT([$_ac_pythondir])

  if test "x$WIN32_PYTHON_INCLUDES" != x; then
    _ac_pyincludes="$WIN32_PYTHON_INCLUDES"
  else
    _ac_pyincludes="-I$WIN32_PYTHON_HOME/include"
  fi

  if test "x$WIN32_PYTHON_LIBS" != x; then
    _ac_pylibs="$WIN32_PYTHON_LIBS"
  else
    _ac_pylibs="-L$WIN32_PYTHON_HOME/libs -lpython$_ac_pyversion_no_dot"
  fi

  if test "x$WIN32_PYTHON_LDFLAGS" != x; then
    _ac_pyldflags="$WIN32_PYTHON_LDFLAGS"
  else
    _ac_pyldflags=
  fi

  _ac_have_pydev=false
  _ac_save_CPPFLAGS="$CPPFLAGS"
  _ac_save_LDFLAGS="$LDFLAGS"
  _ac_save_LIBS="$LIBS"
  CPPFLAGS="$CPPFLAGS $_ac_pyincludes"
  LDFLAGS="$LDFLAGS $_ac_pyldflags"
  LIBS="$LIBS $_ac_pylibs"
  AC_MSG_CHECKING(python headers and linker flags)
  AC_TRY_LINK([#include <Python.h>],[Py_Initialize();],[
    AC_MSG_RESULT([$_ac_pyincludes $_ac_pyldflags $_ac_pylibs])
    _ac_have_pydev=true
  ],[
    AC_MSG_RESULT(not found)
  ])
  CPPFLAGS="$_ac_save_CPPFLAGS"
  LDFLAGS="$_ac_save_LDFLAGS"
  LIBS="$_ac_save_LIBS"

  if $_ac_have_pydev; then
    AC_SUBST(PYTHON_PLATFORM, [nt])
    AC_SUBST(PYTHON_INCLUDES,[$_ac_pyincludes])
    AC_SUBST(PYTHON_LIBS,[$_ac_pylibs])
    AC_SUBST(PYTHON_EXTRA_LIBS,[])
    AC_SUBST(PYTHON_LDFLAGS,[$_ac_pyldflags])
    AC_SUBST(PYTHON_EXTRA_LDFLAGS,[])
    AC_SUBST(pythondir,[$_ac_pythondir])
    AC_SUBST(pyexecdir,[$_ac_pythondir])
    AC_SUBST(pkgpythondir,[\${pythondir}/$PACKAGE])
    AC_SUBST(pkgpyexecdir,[\${pythondir}/$PACKAGE])
    m4_if([$1],[],[:],[$1])
  else
    m4_if([$2],[],[:],[$2])
  fi
])
dnl
dnl end of MOO_AM_PYTHON_DEVEL_CROSS_MINGW
dnl --------------------------------------
