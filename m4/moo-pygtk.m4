##############################################################################
# _MOO_AC_PYGTK_CODEGEN
#
AC_DEFUN([_MOO_AC_PYGTK_CODEGEN],[
    AC_ARG_WITH([custom-codegen], AC_HELP_STRING([--with-custom-codegen], [whether to use custom copy of pygtk codegen (default = yes)]),[
        if test x$with_custom_codegen = "xno"; then
            MOO_USE_CUSTOM_CODEGEN="no"
            AC_MSG_NOTICE([using installed codegen])
        else
            MOO_USE_CUSTOM_CODEGEN="yes"
            AC_MSG_NOTICE([using patched codegen])
        fi
    ],[
        MOO_USE_CUSTOM_CODEGEN="yes"
        AC_MSG_NOTICE([using patched codegen])
    ])

    if test x$MOO_USE_CUSTOM_CODEGEN != xyes; then
        AC_MSG_NOTICE([pygtk codegen dir: $PYGTK_CODEGEN_DIR])
    fi
])


##############################################################################
# _MOO_AC_CHECK_PYGTK_MINGW(version,action-if-found,action-if-not-found)
# checks pygtk stuff for mingw, it's broken
#
AC_DEFUN([_MOO_AC_CHECK_PYGTK_MINGW],[
    # _AC_CHECK_PYGTK_MINGW
    no_dot_version=`echo $1 | sed "s/\.//"`

    if test -z $PYTHON_PARENT_DIR; then
        PYTHON_PARENT_DIR=/usr/local/win
    fi
    if test -z $PYTHON_PREFIX; then
        PYTHON_PREFIX=$PYTHON_PARENT_DIR/Python$no_dot_version
    fi
    if test -z $PYGTK_CFLAGS; then
        PYGTK_CFLAGS="-I$PYTHON_PREFIX/include/pygtk-2.0 $GTK_CFLAGS"
    fi

    dnl check whether pygtk.h exists
    save_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS $PYGTK_CFLAGS -I$PYTHON_PREFIX/include"
    save_CFLAGS="$CFLAGS"
    CFLAGS="$CFLAGS $PYGTK_CFLAGS -I$PYTHON_PREFIX/include"

    AC_MSG_CHECKING([for pygtk headers])
    # start AC_COMPILE_IFELSE in _AC_CHECK_PYGTK_MINGW
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
    #include <pygtk/pygtk.h>
    int main ()
    {
            init_pygtk();
            return 0;
    }]])],[
        AC_MSG_RESULT([$PYGTK_CFLAGS])
        AC_SUBST(PYGTK_CFLAGS)
        PYGTK_DEFS_DIR=$PYTHON_PREFIX/share/pygtk/2.0/defs
        AC_SUBST(PYGTK_DEFS_DIR)
        PYGTK_CODEGEN_DIR=$PYTHON_PREFIX/share/pygtk/2.0/codegen
        AC_SUBST(PYGTK_CODEGEN_DIR)
        AC_MSG_NOTICE([pygtk defs dir: $PYGTK_DEFS_DIR])
        $2
        _MOO_AC_PYGTK_CODEGEN
    ],[
        AC_MSG_RESULT([not found])
        $3
    ])
    # end AC_COMPILE_IFELSE in _AC_CHECK_PYGTK_MINGW

    CFLAGS="$save_CFLAGS"
    CPPFLAGS="$save_CPPFLAGS"
])


##############################################################################
# _MOO_AC_CHECK_PYGTK_UNIX(action-if-found,action-if-not-found)
# checks pygtk stuff
#
AC_DEFUN([_MOO_AC_CHECK_PYGTK_UNIX],[
    # _AC_CHECK_PYGTK_UNIX
    AC_MSG_CHECKING([for pygtk headers])
    PKG_CHECK_MODULES(PYGTK,pygtk-2.0,[
        AC_MSG_RESULT([found])
        AC_MSG_CHECKING([whether pygtk can be used])
        save_CPPFLAGS="$CPPFLAGS"
        CPPFLAGS="$CPPFLAGS $PYGTK_CFLAGS $PYTHON_INCLUDES"
        save_CFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS $PYGTK_CFLAGS $PYTHON_INCLUDES"

        # start AC_COMPILE_IFELSE in _AC_CHECK_PYGTK_UNIX
        AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
        #include <pygobject.h>
        int main ()
        {
            PyObject *object = pygobject_new (NULL);
            return 0;
        }]])],[
            AC_MSG_RESULT(yes)
            $2
            PYGTK_DEFS_DIR=`$PKG_CONFIG --variable=defsdir pygtk-2.0`
            AC_SUBST(PYGTK_DEFS_DIR)
            PYGTK_CODEGEN_DIR=`$PKG_CONFIG --variable=codegendir pygtk-2.0`
            AC_SUBST(PYGTK_CODEGEN_DIR)
            AC_MSG_NOTICE([pygtk defs dir: $PYGTK_DEFS_DIR])
            _MOO_AC_PYGTK_CODEGEN
        ],[
            AC_MSG_RESULT(no)
            $3
        ])
        # end AC_COMPILE_IFELSE in _AC_CHECK_PYGTK_UNIX

        CFLAGS="$save_CFLAGS"
        CPPFLAGS="$save_CPPFLAGS"
    ],[
        AC_MSG_RESULT([not found])
        $3
    ])
])


##############################################################################
# MOO_AC_CHECK_PYGTK(version,action-if-found,action-if-not-found)
# checks pygtk stuff
# version argument is passed to _AC_CHECK_PYGTK_MINGW only
#
AC_DEFUN([MOO_AC_CHECK_PYGTK],[
    AC_REQUIRE([MOO_AC_CHECK_OS])

    if test x$MOO_OS_CYGWIN != xyes; then
        if test x$MOO_OS_MINGW = xyes; then
            _MOO_AC_CHECK_PYGTK_MINGW([$1],[$2],[$3])
        else
            _MOO_AC_CHECK_PYGTK_UNIX([$1],[$2],[$3])
        fi
    fi
])


##############################################################################
# MOO_AC_PYGTK()
#
AC_DEFUN([MOO_AC_PYGTK],[
    AC_REQUIRE([MOO_AC_CHECK_OS])

    AC_ARG_WITH([pygtk], AC_HELP_STRING([--with-pygtk], [whether to compile pygtk support (default = YES)]),[
        if test x$with_pygtk = "xno"; then
            MOO_USE_PYGTK="no"
        else
            MOO_USE_PYGTK="yes"
        fi
    ],[
        MOO_USE_PYGTK="yes"
    ])

    AC_ARG_WITH([python],AC_HELP_STRING([--with-python], [whether to compile python support (default = YES)]),[
        if test x$with_python = "xno"; then
            MOO_USE_PYTHON="no"
            MOO_USE_PYGTK="no"
        else
            if test x$with_python = "xyes"; then
                moo_python_version=2.2
            else
                moo_python_version=$with_python
            fi
            MOO_USE_PYTHON="yes"
        fi
    ],[
        if test x$MOO_USE_PYTGK = xyes; then
            MOO_USE_PYTHON="yes"
        else
            MOO_USE_PYTHON="auto"
            MOO_USE_PYTGK="no"
        fi
        moo_python_version=2.2
    ])

    if test x$MOO_OS_CYGWIN = xyes; then
        MOO_USE_PYTHON=no
        MOO_USE_PYGTK=no
    fi

    if test x$MOO_USE_PYTHON != "xno"; then
        MOO_AC_CHECK_PYTHON($moo_python_version,[
            MOO_USE_PYTHON="yes"
            AC_MSG_NOTICE([compiling python support])
        ],[
            if test x$MOO_USE_PYTHON = "xyes"; then
                AC_MSG_ERROR([Python not found])
            else
                AC_MSG_WARN([Python support will be disabled])
            fi
            MOO_USE_PYTHON="no"
            MOO_USE_PYGTK="no"
        ])
    fi

    AM_CONDITIONAL(MOO_USE_PYTHON, test x$MOO_USE_PYTHON = "xyes")
    if test x$MOO_USE_PYTHON = "xyes"; then
        AC_DEFINE(MOO_USE_PYTHON, 1, [MOO_USE_PYTHON])
    fi

    ### Pygtk

    if test x$MOO_USE_PYGTK = xyes; then
        MOO_AC_CHECK_PYGTK([$moo_python_version],[
            MOO_USE_PYGTK="yes"
            AC_MSG_NOTICE([compiling pygtk support])
        ],[
            MOO_USE_PYGTK="no"
            AC_MSG_NOTICE([usable pygtk not found])
        ])
    fi

    AM_CONDITIONAL(MOO_USE_PYGTK, test x$MOO_USE_PYGTK = "xyes")
    if test x$MOO_USE_PYGTK = "xyes"; then
        AC_DEFINE(MOO_USE_PYGTK, 1, [MOO_USE_PYGTK])

        $PKG_CONFIG --modversion pygtk-2.0
        PYGTK_VERSION=`$PKG_CONFIG --modversion pygtk-2.0`
        i=0
        for part in `echo $PYGTK_VERSION | sed 's/\./ /g'`; do
            i=`expr $i + 1`
            eval part$i=$part
        done

        PYGTK_MAJOR_VERSION=$part1
        PYGTK_MINOR_VERSION=$part2
        PYGTK_MICRO_VERSION=$part3

        AC_SUBST(PYGTK_VERSION)
        AC_SUBST(PYGTK_MAJOR_VERSION)
        AC_SUBST(PYGTK_MINOR_VERSION)
        AC_SUBST(PYGTK_MICRO_VERSION)
    fi

    AM_CONDITIONAL(MOO_USE_CUSTOM_CODEGEN, test x$MOO_USE_CUSTOM_CODEGEN = "xyes")
])
