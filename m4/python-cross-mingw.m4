dnl AM_PYTHON_DEVEL_CROSS_MINGW([action-if-found[,action-if-not-found[,version]]])
AC_DEFUN([AM_PYTHON_DEVEL_CROSS_MINGW],[
  m4_if([$3],[],[
    m4_define([_ac_m4_pyver],[])
  ],[
    m4_define([_ac_m4_pyver],[m4_bpatsubst([$3],[\.])])
  ])

  if test "x$PYTHON[]_ac_m4_pyver[]_HOME" = x; then
    AC_MSG_ERROR([PYTHON[]_ac_m4_pyver[]_HOME environment variable must be set dnl
		  when cross-compiling with mingw])
  fi

  AC_MSG_CHECKING(host system python version)
  m4_if([$3],,[
    if test "x$PYTHON[]_ac_m4_pyver[]_VERSION" = x; then
      # guess python version, very clever heuristics here
      for _ac_python_minor in 3 4 5 6 7 8 9; do
	if test -f "$PYTHON[]_ac_m4_pyver[]_HOME/libs/libpython2$_ac_python_minor.a" -o \
		-f "$PYTHON[]_ac_m4_pyver[]_HOME/libs/python2$_ac_python_minor.lib" ;
	then
	  _ac_pyversion="2.$_ac_python_minor"
	  break
	fi
      done
    else
      _ac_pyversion=$PYTHON[]_ac_m4_pyver[]_VERSION
    fi

    if test "x$_ac_pyversion" = x; then
      AC_MSG_ERROR([Could not determine Python version])
    fi
  ],[
    _ac_pyversion=$3
  ])
  AC_MSG_RESULT([$_ac_pyversion])
  _ac_pyversion_no_dot=`echo $_ac_pyversion | $SED 's/^2\.*\([[3-9]]\).*/2\1/'`

  AC_MSG_CHECKING(installation directory for python modules)
  if test "x$PYTHON[]_ac_m4_pyver[]_PKG_DIR" != x; then
    _ac_pythondir=$PYTHON[]_ac_m4_pyver[]_PKG_DIR
  else
    _ac_pythondir="$PYTHON[]_ac_m4_pyver[]_HOME/Lib/site-packages"
  fi
  AC_MSG_RESULT([$_ac_pythondir])

  if test "x$PYTHON[]_ac_m4_pyver[]_INCLUDES" != x; then
    _ac_pyincludes=$PYTHON[]_ac_m4_pyver[]_INCLUDES
  else
    _ac_pyincludes="-I$PYTHON[]_ac_m4_pyver[]_HOME/include"
  fi

  if test "x$PYTHON[]_ac_m4_pyver[]_LIBS" != x; then
    _ac_pylibs=$PYTHON[]_ac_m4_pyver[]_LIBS
  else
    _ac_pylibs="-L$PYTHON[]_ac_m4_pyver[]_HOME/libs -lpython$_ac_pyversion_no_dot"
  fi

  _ac_have_pydev=false
  _ac_save_CPPFLAGS="$CPPFLAGS"
  _ac_save_LDFLAGS="$LDFLAGS"
  CPPFLAGS="$CPPFLAGS $_ac_pyincludes"
  LDFLAGS="$LDFLAGS $_ac_pylibs"
  AC_MSG_CHECKING(python headers and linker flags)
  dnl AC_TRY_LINK is buggy, it puts libs before source file on compilation
  dnl command line
  AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
  #include <Python.h>
  int main ()
  {
      Py_Initialize();
      return 0;
  }]])],[
    AC_MSG_RESULT([$_ac_pyincludes $_ac_pylibs])
    _ac_have_pydev=true
  ],[
    AC_MSG_RESULT(not found)
  ])
  CPPFLAGS="$_ac_save_CPPFLAGS"
  LDFLAGS="$_ac_save_LDFLAGS"

  if $_ac_have_pydev; then
    AC_SUBST(PYTHON[]_ac_m4_pyver[]_PLATFORM, [nt])
    AC_SUBST(PYTHON[]_ac_m4_pyver[]_INCLUDES,[$_ac_pyincludes])
    AC_SUBST(PYTHON[]_ac_m4_pyver[]_LIBS,[$_ac_pylibs])
    AC_SUBST(PYTHON[]_ac_m4_pyver[]_EXTRA_LIBS,[])
    AC_SUBST(PYTHON[]_ac_m4_pyver[]_LDFLAGS,[])
    AC_SUBST(PYTHON[]_ac_m4_pyver[]_EXTRA_LDFLAGS,[])
    AC_SUBST(python[]_ac_m4_pyver[]dir,[$_ac_pythondir])
    AC_SUBST(pyexec[]_ac_m4_pyver[]dir,[$_ac_pythondir])
    AC_SUBST(pkgpython[]_ac_m4_pyver[]dir,[\${python[]_ac_m4_pyver[]dir}/$PACKAGE])
    AC_SUBST(pkgpyexec[]_ac_m4_pyver[]dir,[\${python[]_ac_m4_pyver[]dir}/$PACKAGE])
    m4_if([$1],[],[:],[$1])
  else
    m4_if([$2],[],[:],[$2])
  fi

  m4_undefine([_ac_m4_pyver])
])
