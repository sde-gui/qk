##############################################################################
# _AC_CHECK_PYTHON_MINGW(version,action-if-found,action-if-not-found)
# checks python stuff when building for mingw. it's broken
#
AC_DEFUN([_AC_CHECK_PYTHON_MINGW],[
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

    # check whether Python.h and library exists

    save_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS $PYTHON_INCLUDES"
    save_CFLAGS="$CPPFLAGS"
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
        $3
    ])

    LDFLAGS="$save_LDFLAGS"
    CFLAGS="$save_CFLAGS"
    CPPFLAGS="$save_CPPFLAGS"
])


##############################################################################
# _AC_PYTHON_DEVEL(action-if-found,action-if-not-found)
# checks python headers and libs. it's
# http://www.gnu.org/software/ac-archive/htmldoc/ac_python_devel.html,
# modified to allow actions if-found/if-not-found
#
AC_DEFUN([_AC_PYTHON_DEVEL],[
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
            python_found=yes
            AC_SUBST([PYTHON_LDFLAGS],["-L$python_path -lpython$PYTHON_VERSION"])
            python_site=`echo $python_path | sed "s/config/site-packages/"`
            AC_SUBST([PYTHON_SITE_PKG],[$python_site])
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
# _AC_CHECK_PYTHON_UNIX(min-version,action-if-found,action-if-not-found)
# checks python stuff when building for unix
#
AC_DEFUN([_AC_CHECK_PYTHON_UNIX],[
    # _AC_CHECK_PYTHON_UNIX
    AM_PATH_PYTHON([$1],[
        _AC_PYTHON_DEVEL([
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
        $3
    fi
])


##############################################################################
# AC_CHECK_PYTHON(min-version,action-if-found,action-if-not-found)
# checks for python, python includes and libs
#
AC_DEFUN([AC_CHECK_PYTHON],[
AC_MSG_NOTICE([checking for headers and libs required to compile python extensions])
if test x$mingw_build = xyes; then
    _AC_CHECK_PYTHON_MINGW([$1],[$2],[$3])
else
    _AC_CHECK_PYTHON_UNIX([$1],[$2],[$3])
fi
])


##############################################################################
# _AC_CHECK_PYGTK_MINGW(version,action-if-found,action-if-not-found)
# checks pygtk stuff for mingw, it's broken
#
AC_DEFUN([_AC_CHECK_PYGTK_MINGW],[
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
    save_CFLAGS="$CPPFLAGS"
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
        AC_MSG_NOTICE([pygtk defs dir: $PYGTK_DEFS_DIR])
        $2
    ],[
        AC_MSG_RESULT([not found])
        $3
    ])
    # end AC_COMPILE_IFELSE in _AC_CHECK_PYGTK_MINGW

    CFLAGS="$save_CFLAGS"
    CPPFLAGS="$save_CPPFLAGS"
])


##############################################################################
# _AC_CHECK_PYGTK_UNIX(action-if-found,action-if-not-found)
# checks pygtk stuff
#
AC_DEFUN([_AC_CHECK_PYGTK_UNIX],[
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
            AC_MSG_NOTICE([pygtk defs dir: $PYGTK_DEFS_DIR])
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
# AC_CHECK_PYGTK(version,action-if-found,action-if-not-found)
# checks pygtk stuff
# version argument is passed to _AC_CHECK_PYGTK_MINGW only
#
AC_DEFUN([AC_CHECK_PYGTK],[
if test x$mingw_build = xyes; then
    _AC_CHECK_PYGTK_MINGW([$1],[$2],[$3])
else
    _AC_CHECK_PYGTK_UNIX([$1],[$2],[$3])
fi
])


##############################################################################
# AC_PROG_WINDRES
# checks whether windres is available, pretty broken
#
AC_DEFUN([AC_PROG_WINDRES],[
case $host in
*-*-mingw*|*-*-cygwin*)
    AC_MSG_CHECKING([for windres])
    found=
    if test -z $WINDRES; then
        if test "x$CC" != "x"; then
            WINDRES=`echo $CC | sed "s/gcc$//"`windres
        else
            WINDRES=windres
        fi
    fi
    found=`$WINDRES --version 2>/dev/null`
    if test "x$found" != "x"; then
        AC_MSG_RESULT([found $WINDRES])
        AC_SUBST(WINDRES)
    else
        AC_MSG_RESULT([not found])
    fi
    ;;
*)
    ;;
esac
])


##############################################################################
# AC_CHECK_XML_STUFF(action-if-found,action-if-not-found)
# checks whether libxml2 is available, checks some functions and structures
#
AC_DEFUN([AC_CHECK_XML_STUFF],[
    PKG_CHECK_MODULES(XML,libxml-2.0,[
        save_CPPFLAGS="$CPPFLAGS"
        CPPFLAGS="$CPPFLAGS $XML_CFLAGS"
        save_CFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS $XML_CFLAGS"
        save_LDFLAGS="$LDFLAGS"
        LDFLAGS="$LDFLAGS $XML_LIBS"

        AC_CHECK_FUNCS(xmlReadFile xmlParseFile)

        AC_MSG_CHECKING([for xmlNode.line])
        AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
        #include <libxml/parser.h>
        #include <libxml/tree.h>
        int main ()
        {
            xmlNode *node;
            int line;
            line = node->line;
            return 0;
        }]])],[
            AC_MSG_RESULT(present)
            AC_DEFINE(HAVE_XMLNODE_LINE,1,[Define if xmlNode structure has 'line' member])
        ],[
            AC_MSG_RESULT(not present)
        ])

        LDFLAGS="$save_LDFLAGS"
        CFLAGS="$save_CFLAGS"
        CPPFLAGS="$save_CPPFLAGS"

        $1
    ],[
        $2
    ])
])


##############################################################################
# AC_CHECK_DEBUG_STUFF(default-compiler-flags)
#
AC_DEFUN([AC_CHECK_DEBUG_STUFF],[
    DEBUG_CFLAGS=$1
    DEBUG_CXXFLAGS=$1

    AC_ARG_ENABLE(debug,
    AC_HELP_STRING([--enable-debug],[enable debug options (default = NO)]),[
        if test "x$enable_debug" = "xno"; then
            debug="no"
        else
            debug="yes"
        fi
    ],[
        debug="no"
    ])

    AC_ARG_ENABLE(all-gcc-warnings,
    AC_HELP_STRING([--enable-all-gcc-warnings],[enable most of gcc warnings and turn on -pedantic mode (default = NO)]),[
        if test "x$enable_all_gcc_warnings" = "xno"; then
            all_gcc_warnings="no"
        elif test "x$enable_all_gcc_warnings" = "xfatal"; then
            all_gcc_warnings="yes"
            warnings_fatal="yes"
        else
            all_gcc_warnings="yes"
            warnings_fatal="no"
        fi
    ],[
        all_gcc_warnings="no"
    ])

    AC_ARG_ENABLE(all-intel-warnings,
    AC_HELP_STRING([--enable-all-intel-warnings], [enable most of intel compiler warnings (default = NO)]),[
        if test x$enable_all_intel_warnings = "xno"; then
            all_intel_warnings="no"
        else
            all_intel_warnings="yes"
        fi
    ],[
        all_intel_warnings="no"
    ])

    if test x$all_intel_warnings = "xyes"; then
        DEBUG_CFLAGS="$DEBUG_CFLAGS -Wall -Wcheck -w2"
        DEBUG_CXXFLAGS="$DEBUG_CXXFLAGS -Wall -Wcheck -w2 -wd981,279,858,1418 -wd383"
    fi

    if test x$all_gcc_warnings = "xyes"; then
DEBUG_CFLAGS="$DEBUG_CFLAGS -W -Wall -Wpointer-arith dnl
-std=c99 -Wcast-align -Wsign-compare -Winline -Wreturn-type dnl
-Wwrite-strings -Wmissing-prototypes -Wmissing-declarations dnl
-Wmissing-noreturn -Wmissing-format-attribute -Wnested-externs dnl
-Wunreachable-code -Wdisabled-optimization"
DEBUG_CXXFLAGS="$DEBUG_CXXFLAGS -W -Wall -Woverloaded-virtual dnl
-Wpointer-arith -Wcast-align -Wsign-compare -Wnon-virtual-dtor dnl
-Wno-long-long -Wundef -Wconversion -Wchar-subscripts -Wwrite-strings dnl
-Wmissing-format-attribute -Wcast-align -Wdisabled-optimization dnl
-Wnon-template-friend -Wsign-promo -Wno-ctor-dtor-privacy"
    fi

    if test x$debug = "xyes"; then
        DEBUG_CFLAGS="$DEBUG_CFLAGS -DG_DISABLE_DEPRECATED -DDEBUG dnl
-DENABLE_DEBUG -DENABLE_PROFILE -DG_ENABLE_DEBUG -DG_ENABLE_PROFILE"
        DEBUG_CXXFLAGS="$DEBUG_CXXFLAGS -DG_DISABLE_DEPRECATED -DDEBUG dnl
-DENABLE_DEBUG -DENABLE_PROFILE -DG_ENABLE_DEBUG -DG_ENABLE_PROFILE"
    else
        DEBUG_CFLAGS="$DEBUG_CFLAGS -DNDEBUG=1 -DG_DISABLE_CAST_CHECKS -DG_DISABLE_ASSERT"
        DEBUG_CXXFLAGS="$DEBUG_CXXFLAGS -DNDEBUG=1 -DG_DISABLE_CAST_CHECKS -DG_DISABLE_ASSERT"
    fi

    if test "x$all_gcc_warnings" = "xyes" -a "x$warnings_fatal" = "xyes"; then
        DEBUG_CXXFlAGS_NO_WERROR=$DEBUG_CXXFLAGS
        DEBUG_CXXFLAGS="-Werror $DEBUG_CXXFLAGS"
    fi

    AC_SUBST(DEBUG_CFLAGS)
    AC_SUBST(DEBUG_CXXFLAGS)
    AC_SUBST(DEBUG_CXXFlAGS_NO_WERROR)

    AC_MSG_CHECKING(for C compiler debug options)
    if test "x$DEBUG_CFLAGS" = "x"; then
        AC_MSG_RESULT(None)
    else
        AC_MSG_RESULT($DEBUG_CFLAGS)
    fi
    AC_MSG_CHECKING(for C++ compiler debug options)
    if test "x$DEBUG_CXXFLAGS" = "x"; then
        AC_MSG_RESULT(None)
    else
        AC_MSG_RESULT($DEBUG_CXXFLAGS)
    fi
])


##############################################################################
# _CHECK_VERSION(pkg-name)
#
AC_DEFUN([_CHECK_VERSION],[
    if test x$cygwin_build != xyes; then
        PKG_CHECK_MODULES($1,$2)

        AC_MSG_CHECKING($1 version)
        $1[]_VERSION=`$PKG_CONFIG --modversion $2`

        i=0
        for part in `echo $[]$1[]_VERSION | sed 's/\./ /g'`; do
            i=`expr $i + 1`
            eval part$i=$part
        done

        $1[]_MAJOR_VERSION=$part1
        $1[]_MINOR_VERSION=$part2
        $1[]_MICRO_VERSION=$part3

        if test $[]$1[]_MINOR_VERSION -ge 6; then
            ver_2_6=yes
        fi
        if test $[]$1[]_MINOR_VERSION -ge 4; then
            ver_2_4=yes
        fi
        if test $[]$1[]_MINOR_VERSION -ge 2; then
            ver_2_2=yes
        fi

        AC_MSG_RESULT($[]$1[]_MAJOR_VERSION.$[]$1[]_MINOR_VERSION.$[]$1[]_MICRO_VERSION)
    fi

    AM_CONDITIONAL($1[]_2_6, test x$ver_2_6 = "xyes")
    AM_CONDITIONAL($1[]_2_4, test x$ver_2_4 = "xyes")
    AM_CONDITIONAL($1[]_2_2, test x$ver_2_2 = "xyes")
])


##############################################################################
# PKG_CHECK_GTK_VERSIONS
#
AC_DEFUN([PKG_CHECK_GTK_VERSIONS],[
    _CHECK_VERSION(GTK, gtk+-2.0)
    _CHECK_VERSION(GLIB, glib-2.0)
    _CHECK_VERSION(GDK, gdk-2.0)
])
