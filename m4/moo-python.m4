##############################################################################
# _MOO_AC_CHECK_PYTHON_MINGW(version,action-if-found,action-if-not-found)
# checks python stuff when building for mingw. it's broken
#
AC_DEFUN([_MOO_AC_CHECK_PYTHON_MINGW],[
    no_dot_version=`echo $1 | sed "s/\.//"`

    if test -z $PYTHON_PARENT_DIR; then
        PYTHON_PARENT_DIR=/usr/local/win
    fi
    if test -z $PYTHON_PREFIX; then
        PYTHON_PREFIX=$PYTHON_PARENT_DIR/Python$no_dot_version
    fi
    if test -z $PYTHON_INCLUDES; then
        PYTHON_INCLUDES="-I$PYTHON_PREFIX/include -mno-cygwin"
    fi
    if test -z $PYTHON_LDFLAGS; then
        PYTHON_LDFLAGS="-L$PYTHON_PREFIX/libs -lpython$no_dot_version -mno-cygwin"
    fi
    if test -z $PYTHON; then
        PYTHON="python"
    fi

    # check whether Python.h and library exists

    save_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS $PYTHON_INCLUDES"
    save_CFLAGS="$CFLAGS"
    CFLAGS="$CFLAGS $PYTHON_INCLUDES"
    save_LDFLAGS="$LDFLAGS"
    LDFLAGS="$LDFLAGS $PYTHON_LDFLAGS"

    AC_MSG_CHECKING([PYTHON_INCLUDES])
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
        AC_MSG_RESULT([$PYTHON_INCLUDES])
        AC_MSG_CHECKING([PYTHON_LDFLAGS])
        AC_MSG_RESULT([$PYTHON_LDFLAGS])
        AC_MSG_NOTICE([Did not do real linking])
        AC_SUBST(PYTHON_INCLUDES)
        AC_SUBST(PYTHON_LDFLAGS)
        pyexecdir=$PYTHON_PREFIX/Lib/site-packages
        $2
    ],[
        AC_MSG_RESULT([Not found])
        PYTHON_INCLUDES=""
        PYTHON_LDFLAGS=""
        PYTHON_EXTRA_LIBS=""
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
# modified to allow actions if-found/if-not-found;
# also it uses distutils' LDFLAGS - at least on freebsd it must be used
# to pick up -pthread option.
#
AC_DEFUN([_MOO_AC_PYTHON_DEVEL],[
    # Check for Python include path
    # if PYTHON_INCLUDES is set, do not do anything
    if test -z $PYTHON_INCLUDES; then
        python_found=no
        python_path=`echo $PYTHON | sed "s,/bin.*$,,"`
        for i in "$python_path/include/python$PYTHON_VERSION/" "$python_path/include/python/" ; do
            if test -e "$i/Python.h"; then
                python_found=yes
                AC_MSG_CHECKING([for python include path])
                AC_MSG_RESULT([$i])
                PYTHON_INCLUDES="-I$i"
                AC_SUBST([PYTHON_INCLUDES],[$PYTHON_INCLUDES])
                break
            fi
        done
        if test x$python_found = xno; then
            AC_MSG_CHECKING([for python include path])
            AC_MSG_RESULT([Not found])
        fi
    else
        python_found=yes
        AC_SUBST([PYTHON_INCLUDES],[$PYTHON_INCLUDES])
    fi

    if test x$python_found = xyes; then
        # Check for Python library path
        AC_MSG_CHECKING([for Python library path])
        python_path=`echo $PYTHON | sed "s,/bin.*$,,"`
        for i in "$python_path/lib/python$PYTHON_VERSION/config/" "$python_path/lib/python$PYTHON_VERSION/" "$python_path/lib/python/config/" "$python_path/lib/python/" "$python_path/" ; do
            python_path=`find $i -type f -name libpython$PYTHON_VERSION.* -print | sed "1q"`
            if test -n "$python_path" ; then
                break
            fi
        done
        python_path=`echo $python_path | sed "s,/libpython.*$,,"`
        AC_MSG_RESULT([$python_path])
        if test -z "$python_path" ; then
            python_found=no
        else
            AC_MSG_CHECKING(python linker flags)

            python_found=yes
            PYTHON_LDFLAGS="-L$python_path -lpython$PYTHON_VERSION"
            python_site=`echo $python_path | sed "s/config/site-packages/"`
            AC_SUBST([PYTHON_SITE_PKG],[$python_site])

            # this picks up -pthread option on FreeBSD
            PYTHON_EXTRA_LDFLAGS=`$PYTHON -c "import distutils.sysconfig; \
                    conf = distutils.sysconfig.get_config_var; \
                    print conf('LDFLAGS')"`

            PYTHON_LDFLAGS="$PYTHON_LDFLAGS $PYTHON_EXTRA_LDFLAGS"
            AC_MSG_RESULT($PYTHON_LDFLAGS)
            AC_SUBST(PYTHON_LDFLAGS)
        fi
    fi

    if test x$python_found = xyes; then
        # libraries which must be linked in when embedding
        AC_MSG_CHECKING(python extra libraries)
        PYTHON_EXTRA_LIBS=`$PYTHON -c "import distutils.sysconfig; \
                conf = distutils.sysconfig.get_config_var; \
                print conf('LOCALMODLIBS')+' '+conf('LIBS')"
        AC_MSG_RESULT($PYTHON_EXTRA_LIBS)`
        AC_SUBST(PYTHON_EXTRA_LIBS)

        AC_MSG_CHECKING([whether python can be used])

        save_CPPFLAGS="$CPPFLAGS"
        CPPFLAGS="$CPPFLAGS $PYTHON_INCLUDES"
        save_CFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS $PYTHON_INCLUDES"

        # start AC_COMPILE_IFELSE in _AC_PYTHON_DEVEL
        AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
        #include <Python.h>
        int main ()
        {
            PyObject *object = NULL;
            return 0;
        }]])],[
            python_found=yes
            AC_MSG_RESULT(yes)
            AC_MSG_NOTICE([$PYTHON_INCLUDES $PYTHON_LDFLAGS $PYTHON_EXTRA_LIBS])
        ],[
            python_found=no
            AC_MSG_RESULT(no)
        ])
        # end AC_COMPILE_IFELSE in _AC_PYTHON_DEVEL

        CFLAGS="$save_CFLAGS"
        CPPFLAGS="$save_CPPFLAGS"
    fi

    if test x$python_found = xyes; then
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
        PYTHON_INCLUDES=""
        PYTHON_LDFLAGS=""
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
            _MOO_AC_CHECK_PYTHON_MINGW([$1],[$2],[$3])
        else
            _MOO_AC_CHECK_PYTHON_UNIX([$1],[$2],[$3])
        fi
    fi
])
