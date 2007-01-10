##############################################################################
# _MOO_AC_PYTHON_DEVEL(action-if-found,action-if-not-found)
# checks python headers and libs. it's
# http://www.gnu.org/software/ac-archive/htmldoc/ac_python_devel.html,
# modified to allow actions if-found/if-not-found
#
AC_DEFUN([_MOO_AC_PYTHON_DEVEL],[
    # Check for distutils first
    AC_MSG_CHECKING([for the distutils Python package])
    $PYTHON -c "import distutils" 2>/dev/null
    if test $? -eq 0; then
        python_found=yes
    	AC_MSG_RESULT([yes])
    else
        python_found=no
    	AC_MSG_RESULT([no])
    	AC_MSG_ERROR([cannot import Python module "distutils".
Please check your Python installation.])
    fi

    # Check for Python include path
    # if PYTHON_INCLUDES is set, do not do anything
    if test $python_found = yes; then
	AC_MSG_CHECKING([for Python include path])

        if test -z "$PYTHON_INCLUDES"; then
            python_path=`$PYTHON -c "import distutils.sysconfig; \
                                     print distutils.sysconfig.get_python_inc();"`
            if test -n "${python_path}"; then
                python_path="-I$python_path"
            fi
            PYTHON_INCLUDES=$python_path
        fi

        AC_MSG_RESULT([$PYTHON_INCLUDES])
	AC_SUBST([PYTHON_INCLUDES])
    fi

    # Check for Python linker flags
    # if PYTHON_LIBS is set, do not do anything
    if test $python_found = yes; then
	AC_MSG_CHECKING([Python linker flags])

        if test "x$PYTHON_LIBS" = "x"; then
            # (makes two attempts to ensure we've got a version number
            # from the interpreter)
            py_version=`$PYTHON -c "from distutils.sysconfig import *; \
                        from string import join; \
                        print join(get_config_vars('VERSION'))"`
            if test "x$py_version" = "x[None]"; then
                if test "x$PYTHON_VERSION" != "x"; then
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

        if test "x$PYTHON_EXTRA_LIBS" = "x"; then
            PYTHON_EXTRA_LIBS=`$PYTHON -c "import distutils.sysconfig; \
                                           conf = distutils.sysconfig.get_config_var; \
                                           print conf('LOCALMODLIBS'), conf('LIBS')"`
            PYTHON_EXTRA_LDFLAGS=`$PYTHON -c "import distutils.sysconfig; \
                                              conf = distutils.sysconfig.get_config_var; \
                                              print conf('LDFLAGS')"`
        fi

	AC_MSG_CHECKING([Python extra libs])
        AC_MSG_RESULT([$PYTHON_EXTRA_LIBS])
	AC_MSG_CHECKING([Python extra linker flags])
        AC_MSG_RESULT([$PYTHON_EXTRA_LDFLAGS])
	AC_SUBST([PYTHON_EXTRA_LIBS])
	AC_SUBST([PYTHON_EXTRA_LDFLAGS])
    fi

    if test $python_found = yes; then
        m4_if([$1],[],[:],[$1])
    else
        m4_if([$2],[],[:],[$2])
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
        m4_if([$2],[],[:],[$2])
    else
        PYTHON_INCLUDES=""
        PYTHON_LIBS=""
        PYTHON_EXTRA_LIBS=""
        m4_if([$3],[],[:],[$3])
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
          AM_PYTHON_DEVEL_CROSS_MINGW([$2],[$3],[2.4])
          AM_PYTHON_DEVEL_CROSS_MINGW([$2],[$3],[2.5])
        else
          _MOO_AC_CHECK_PYTHON_UNIX([$1],[$2],[$3])
        fi
    fi
])
