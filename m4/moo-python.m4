##############################################################################
# _MOO_AC_CHECK_PYTHON_MINGW(version,action-if-found,action-if-not-found)
# checks python stuff when building for mingw. it's broken
#
AC_DEFUN([_MOO_AC_CHECK_PYTHON_MINGW],[
    if test -z "$PYTHON[]$1[]_PARENT_DIR"; then
        PYTHON[]$1[]_PARENT_DIR=/usr/local/win
    fi
    if test -z "$PYTHON[]$1[]_PREFIX"; then
        PYTHON[]$1[]_PREFIX=$PYTHON[]$1[]_PARENT_DIR/Python$1
    fi
    if test -z "$PYTHON[]$1[]_CFLAGS"; then
        PYTHON[]$1[]_CFLAGS="-I$PYTHON[]$1[]_PREFIX/include -mno-cygwin"
    fi
    if test -z "$PYTHON[]$1[]_LIBS"; then
        PYTHON[]$1[]_LIBS="-L$PYTHON[]$1[]_PREFIX/libs -lpython$1 -mno-cygwin"
    fi
    if test -z "$PYTHON[]$1"; then
        PYTHON[]$1="python"
    fi

    PYTHON_PARENT_DIR="$PYTHON[]$1[]_PARENT_DIR"
    PYTHON_PREFIX="$PYTHON[]$1[]_PREFIX"
    PYTHON="$PYTHON[]$1[]"

    # check whether Python.h and library exists

    save_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS $PYTHON[]$1[]_CFLAGS"
    save_CFLAGS="$CFLAGS"
    CFLAGS="$CFLAGS $PYTHON[]$1[]_CFLAGS"
    save_LDFLAGS="$LDFLAGS"
    LDFLAGS="$LDFLAGS $PYTHON[]$1[]_LIBS"

    AC_MSG_CHECKING([PYTHON[]$1[]_CFLAGS])
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
    #include <Python.h>
    int main ()
    {
        Py_Initialize();
        PyRun_SimpleString("from time import time,ctime\n"
                            "print 'Today is',ctime(time())\n");
        Py_Finalize();
        return 0;
    }]])],[
        AC_MSG_RESULT([$PYTHON[]$1[]_CFLAGS])
        AC_MSG_CHECKING([PYTHON[]$1[]_LIBS])
        AC_MSG_RESULT([$PYTHON[]$1[]_LIBS])
        AC_MSG_NOTICE([Did not do real linking])
        AC_SUBST(PYTHON[]$1[]_CFLAGS)
        AC_SUBST(PYTHON[]$1[]_LIBS)
        pyexecdir=$PYTHON[]$1[]_PREFIX/Lib/site-packages
        $2
    ],[
        AC_MSG_RESULT([Not found])
        PYTHON[]$1[]_CFLAGS=""
        PYTHON[]$1[]_LIBS=""
        PYTHON[]$1[]_EXTRA_LIBS=""
        $3
    ])

    LDFLAGS="$save_LDFLAGS"
    CFLAGS="$save_CFLAGS"
    CPPFLAGS="$save_CPPFLAGS"
])


##############################################################################
# _MOO_AC_PYTHON_DEVEL(action-if-found,action-if-not-found)
# checks python headers and libs. it's
# http://www.gnu.org/software/ac-archive/htmldoc/ac_python_devel.html,
# modified to allow actions if-found/if-not-found
#
AC_DEFUN([_MOO_AC_PYTHON_DEVEL],[
    # Check for distutils first
    AC_MSG_CHECKING([for the distutils Python package])
    ac_distutils_result=`$PYTHON -c "import distutils" 2>&1`
    if test -z "$ac_distutils_result"; then
        python_found=yes
    	AC_MSG_RESULT([yes])
    else
        python_found=no
    	AC_MSG_RESULT([no])
    	AC_MSG_ERROR([cannot import Python module "distutils".
Please check your Python installation. The error was:
$ac_distutils_result])
    fi

    # Check for Python include path
    # if PYTHON_CFLAGS is set, do not do anything
    if test $python_found = yes; then
	AC_MSG_CHECKING([for Python include path])

        if test -z "$PYTHON_CFLAGS"; then
            python_path=`$PYTHON -c "import distutils.sysconfig; \
                                     print distutils.sysconfig.get_python_inc();"`
            if test -n "${python_path}"; then
                python_path="-I$python_path"
            fi
            PYTHON_CFLAGS=$python_path
        fi

        AC_MSG_RESULT([$PYTHON_CFLAGS])
	AC_SUBST([PYTHON_CFLAGS])
    fi

    # Check for Python linker flags
    # if PYTHON_LIBS is set, do not do anything
    if test $python_found = yes; then
	AC_MSG_CHECKING([Python linker flags])

        if test -z "$PYTHON_LIBS"; then
            # (makes two attempts to ensure we've got a version number
            # from the interpreter)
            py_version=`$PYTHON -c "from distutils.sysconfig import *; \
                        from string import join; \
                        print join(get_config_vars('VERSION'))"`
            if test "$py_version" == "[None]"; then
                if test -n "$PYTHON_VERSION"; then
                    py_version=$PYTHON_VERSION
                else
                    py_version=`$PYTHON -c "import sys; \
                                print sys.version[[:3]]"`
                fi
            fi

            PYTHON_LIBS=`$PYTHON -c "from distutils.sysconfig import *; \
                                     from string import join; \
                                     print '-L' + get_python_lib(0,1), \
                                     '-lpython';"`$py_version
        fi

        AC_MSG_RESULT([$PYTHON_LIBS])
	AC_SUBST([PYTHON_LIBS])
    fi

    # Check for Python extra linker flags
    # if PYTHON_EXTRA_LIBS is set, do not do anything
    if test $python_found = yes; then

        if test -z "$PYTHON_EXTRA_LIBS"; then
            PYTHON_EXTRA_LIBS=`$PYTHON -c "import distutils.sysconfig; \
                                           conf = distutils.sysconfig.get_config_var; \
                                           print conf('LOCALMODLIBS'), conf('LIBS')"`
            PYTHON_EXTRA_LDFLAGS=`$PYTHON -c "import distutils.sysconfig; \
                                              conf = distutils.sysconfig.get_config_var; \
                                              print conf('LINKFORSHARED'), conf('LDFLAGS')"`
        fi

	AC_MSG_CHECKING([Python extra libs])
        AC_MSG_RESULT([$PYTHON_EXTRA_LIBS])
	AC_MSG_CHECKING([Python extra linker flags])
        AC_MSG_RESULT([$PYTHON_EXTRA_LDFLAGS])
	AC_SUBST([PYTHON_EXTRA_LIBS])
	AC_SUBST([PYTHON_EXTRA_LDFLAGS])
    fi

    if test $python_found = yes; then
        $1
    else
        $2
    fi
])


##############################################################################
# _MOO_AC_CHECK_PYTHON_UNIX(min-version,action-if-found,action-if-not-found)
# checks python stuff when building for unix
#
AC_DEFUN([_MOO_AC_CHECK_PYTHON_UNIX],[
    AM_PATH_PYTHON([$1],[
        _MOO_AC_PYTHON_DEVEL([
            python_found=yes
        ],[
            AC_MSG_WARN([Found python interpreter but no development headers or libraries])
            python_found=no
        ])
    ],[
        python_found=no
    ])

    if test x$python_found = xyes; then
        $2
    else
        PYTHON_CFLAGS=""
        PYTHON_LIBS=""
        PYTHON_EXTRA_LIBS=""
        $3
    fi
])


##############################################################################
# MOO_AC_CHECK_PYTHON(min-version,action-if-found,action-if-not-found)
# checks for python, python includes and libs
#
AC_DEFUN([MOO_AC_CHECK_PYTHON],[
AC_MSG_NOTICE([checking for headers and libs required to compile python extensions])
    AC_REQUIRE([MOO_AC_CHECK_OS])
    if test x$MOO_OS_CYGWIN != xyes; then
        if test x$MOO_OS_MINGW = xyes; then
            _MOO_AC_CHECK_PYTHON_MINGW([23],[$2],[$3])
            _MOO_AC_CHECK_PYTHON_MINGW([24],[$2],[$3])
        else
            _MOO_AC_CHECK_PYTHON_UNIX([$1],[$2],[$3])
        fi
    fi
])
