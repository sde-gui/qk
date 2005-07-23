dnl Available from the GNU Autoconf Macro Archive at:
dnl http://www.gnu.org/software/ac-archive/htmldoc/ac_python_devel.html
dnl
AC_DEFUN([AC_PYTHON_DEVEL_NO_AM_PATH_PYTHON],[
    # Check for Python include path
    AC_MSG_CHECKING([for Python include path])
    python_path=`echo $PYTHON | sed "s,/bin.*$,,"`
    for i in "$python_path/include/python$PYTHON_VERSION/" "$python_path/include/python/" "$python_path/" ; do
        python_path=`find $i -type f -name Python.h -print | sed "1q"`
        if test -n "$python_path" ; then
            break
        fi
    done
    python_path=`echo $python_path | sed "s,/Python.h$,,"`
    AC_MSG_RESULT([$python_path])
    if test -z "$python_path" ; then
        AC_MSG_NOTICE([cannot find Python include path])
        $2
    fi
    AC_SUBST([PYTHON_INCLUDES],[-I$python_path])

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
        AC_MSG_NOTICE([cannot find Python library path])
        $2
    fi
    AC_SUBST([PYTHON_LDFLAGS],["-L$python_path -lpython$PYTHON_VERSION"])
    #
    python_site=`echo $python_path | sed "s/config/site-packages/"`
    AC_SUBST([PYTHON_SITE_PKG],[$python_site])
    #
    # libraries which must be linked in when embedding
    #
    AC_MSG_CHECKING(python extra libraries)
    PYTHON_EXTRA_LIBS=`$PYTHON -c "import distutils.sysconfig; \
        conf = distutils.sysconfig.get_config_var; \
        print conf('LOCALMODLIBS')+' '+conf('LIBS')"
    AC_MSG_RESULT($PYTHON_EXTRA_LIBS)`
    AC_SUBST(PYTHON_EXTRA_LIBS)

    $1
])


AC_DEFUN([AC_CHECK_PYTHON],[
    AC_MSG_NOTICE([checking for headers and libs required to compile python extensions])
    if test x$mingw_build = xyes; then

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

        dnl check whether Python.h and library exists
        save_CPPFLAGS="$CPPFLAGS"
        CPPFLAGS="$CPPFLAGS $PYTHON_INCLUDES"
        save_CFLAGS="$CPPFLAGS"
        CFLAGS="$CFLAGS $PYTHON_INCLUDES"
        save_LDFLAGS="$LDFLAGS"
        LDFLAGS="$LDFLAGS $PYTHON_LDFLAGS"
        AC_LANG_PUSH(C)

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
            }]])],
            [python_can_compile=yes],
            [python_can_compile=no])

        if test x$python_can_compile = x"yes"; then
            AC_MSG_RESULT([$PYTHON_INCLUDES])
            AC_MSG_CHECKING([PYTHON_LDFLAGS])
            AC_MSG_RESULT([$PYTHON_LDFLAGS])
            AC_MSG_NOTICE([Did not do real linking])
            AC_SUBST(PYTHON_INCLUDES)
            AC_SUBST(PYTHON_LDFLAGS)
            found_python="yes"
            pyexecdir=$PYTHON_PREFIX/Lib/site-packages
            $2
        else
            AC_MSG_RESULT([Python.h not found])
            $3
        fi

        AC_LANG_POP(C)
        LDFLAGS="$save_LDFLAGS"
        CFLAGS="$save_CFLAGS"
        CPPFLAGS="$save_CPPFLAGS"

    else # mingw_build == "no"

        AM_PATH_PYTHON($1,[
            AC_MSG_NOTICE([found python interpreter $PYTHON])
            AC_PYTHON_DEVEL_NO_AM_PATH_PYTHON([
                AC_MSG_NOTICE([found python libs and headers])
                found_python="yes"
                $2
            ],[
                AC_MSG_NOTICE([python libs and headers not found])
                found_python="no"
                $3
            ])
        ],[
            AC_MSG_NOTICE([python interpreter not found])
            found_python="no"
            $3
        ])

        if test x$found_python = x"yes"; then
            AC_MSG_NOTICE([$PYTHON_INCLUDES $PYTHON_LDFLAGS $PYTHON_EXTRA_LIBS])
        fi
    fi # mingw_build == "no"
])


AC_DEFUN([AC_CHECK_PYGTK],[
    if test x$mingw_build = xyes; then
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
        AC_LANG_PUSH(C)

        AC_MSG_CHECKING([for pygtk headers])
        AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
            #include <pygtk/pygtk.h>
            int main ()
            {
                init_pygtk();
                return 0;
            }]])],
            [pygtk_can_compile=yes],
            [pygtk_can_compile=no])

        if test x$pygtk_can_compile = x"yes"; then
            AC_MSG_RESULT([$PYGTK_CFLAGS])
            AC_SUBST(PYGTK_CFLAGS)
            $2
            PYGTK_DEFS_DIR=$PYTHON_PREFIX/share/pygtk/2.0/defs
            AC_SUBST(PYGTK_DEFS_DIR)
            AC_MSG_NOTICE([pygtk defs dir: $PYGTK_DEFS_DIR])
        else
            AC_MSG_RESULT([not found])
            $3
        fi

        AC_LANG_POP(C)
        CFLAGS="$save_CFLAGS"
        CPPFLAGS="$save_CPPFLAGS"

    else # mingw_build == "no"
        AC_MSG_CHECKING([for pygtk headers])

        PKG_CHECK_MODULES(PYGTK, pygtk-2.0,[
            AC_MSG_RESULT([found])

            AC_MSG_CHECKING([whether pygtk can be used])
            save_CPPFLAGS="$CPPFLAGS"
            CPPFLAGS="$CPPFLAGS $PYGTK_CFLAGS $PYTHON_INCLUDES"
            save_CXXFLAGS="$CXXFLAGS"
            CXXFLAGS="$CXXFLAGS $PYGTK_CFLAGS $PYTHON_INCLUDES"
            AC_LANG_PUSH(C++)
            AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
                #include <pygobject.h>
                int main ()
                {
                    PyObject *object = pygobject_new (NULL);
                    return 0;
                }]])],
                [pygtk_can_compile=yes],
                [pygtk_can_compile=no])
            if test "x$pygtk_can_compile" = "xyes"; then
                AC_MSG_RESULT(yes)
                $2
                PYGTK_DEFS_DIR=`$PKG_CONFIG --variable=defsdir pygtk-2.0`
                AC_SUBST(PYGTK_DEFS_DIR)
                AC_MSG_NOTICE([pygtk defs dir: $PYGTK_DEFS_DIR])
            else
                AC_MSG_RESULT(no)
                $3
            fi
            AC_LANG_POP(C++)
            CXXFLAGS="$save_CXXFLAGS"
            CPPFLAGS="$save_CPPFLAGS"
        ],[
            AC_MSG_RESULT([not found])
            $3
        ])
    fi # mingw_build == "no"
])


AC_DEFUN([AC_PROG_WINDRES],[
case $host in
*-*-mingw*|*-*-cygwin*)
    AC_MSG_CHECKING([for windres])
    found=
    if test "x$WINDRES" = "x"; then
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


AC_DEFUN([AC_CHECK_XML_STUFF],[
    PKG_CHECK_MODULES(XML, libxml-2.0,[
        save_CPPFLAGS="$CPPFLAGS"
        CPPFLAGS="$CPPFLAGS $XML_CFLAGS"
        save_CFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS $XML_CFLAGS"
        save_LDFLAGS="$LDFLAGS"
        LDFLAGS="$LDFLAGS $XML_LDFLAGS"
        AC_LANG_PUSH(C++)

        AC_MSG_CHECKING([for xmlParseFile])
        AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
            #include <libxml/parser.h>
            #include <libxml/tree.h>
            int main ()
            {
                xmlDoc *doc;
                doc = xmlParseFile ("filename");
                return 0;
            }]])],
            [xmlparsefile_can_compile=yes],
            [xmlparsefile_can_compile=no])
        if test "x$xmlparsefile_can_compile" = "xyes"; then
            AC_MSG_RESULT(found)
            AC_DEFINE(HAVE_XMLPARSEFILE,1,[Define if libxml2 defines xmlParseFile])
        else
            AC_MSG_RESULT(not found)
        fi

        AC_MSG_CHECKING([for xmlReadFile])
        AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
            #include <libxml/parser.h>
            #include <libxml/tree.h>
            int main ()
            {
                xmlDoc *doc;
                doc = xmlReadFile ("filename", NULL, 0);
                return 0;
            }]])],
            [xmlreadfile_can_compile=yes],
            [xmlreadfile_can_compile=no])
        if test "x$xmlreadfile_can_compile" = "xyes"; then
            AC_MSG_RESULT(found)
            AC_DEFINE(HAVE_XMLREADFILE,1,[Define if libxml2 defines xmlReadFile])
        else
            AC_MSG_RESULT(not found)
        fi

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
            }]])],
            [xmlnode_line_can_compile=yes],
            [xmlnode_line_can_compile=no])
        if test "x$xmlnode_line_can_compile" = "xyes"; then
            AC_MSG_RESULT(present)
            AC_DEFINE(HAVE_XMLNODE_LINE,1,[Define if xmlNode structure has 'line' member])
        else
            AC_MSG_RESULT(not present)
        fi

        AC_LANG_POP(C++)
        LDFLAGS="$save_LDFLAGS"
        CFLAGS="$save_CFLAGS"
        CPPFLAGS="$save_CPPFLAGS"

        $1
    ],[
        $2
    ])
])


AC_DEFUN([AC_CHECK_DEBUG_STUFF],[
    M_CFLAGS=$1
    M_CXXFLAGS=$1

    AC_ARG_ENABLE(debug,
    AC_HELP_STRING([--enable-debug], [enable debug options (default = NO)]),
    [
        if test "x$enable_debug" = "xno"; then
            debug="no"
        else
            debug="yes"
        fi
    ], [
        debug="no"
    ])

    AC_ARG_ENABLE(all-gcc-warnings,
    AC_HELP_STRING([--enable-all-gcc-warnings], [enable most of gcc warnings and turn on -pedantic mode (default = NO)]),
    [
        if test "x$enable_all_gcc_warnings" = "xno"; then
            all_gcc_warnings="no"
        elif test "x$enable_all_gcc_warnings" = "xfatal"; then
            all_gcc_warnings="yes"
            warnings_fatal="yes"
        else
            all_gcc_warnings="yes"
            warnings_fatal="no"
        fi
    ], [
        all_gcc_warnings="no"
    ])

    AC_ARG_ENABLE(all-intel-warnings,
    AC_HELP_STRING([--enable-all-intel-warnings], [enable most of intel compiler warnings (default = NO)]),
    [
        if test x$enable_all_intel_warnings = "xno"; then
            all_intel_warnings="no"
        else
            all_intel_warnings="yes"
        fi
    ], [
        all_intel_warnings="no"
    ])

    if test x$all_intel_warnings = "xyes"; then
        M_CFLAGS="$M_CFLAGS -Wall -Wcheck -w2"
        M_CXXFLAGS="$M_CXXFLAGS -Wall -Wcheck -w2 -wd981,279,858,1418 -wd383"
    fi

    if test x$all_gcc_warnings = "xyes"; then
M_CFLAGS="$M_CFLAGS -W -Wall -Wpointer-arith dnl
-std=c99 -Wcast-align -Wsign-compare -Winline -Wreturn-type dnl
-Wwrite-strings -Wmissing-prototypes -Wmissing-declarations dnl
-Wmissing-noreturn -Wmissing-format-attribute -Wnested-externs dnl
-Wunreachable-code -Wdisabled-optimization"
M_CXXFLAGS="$M_CXXFLAGS -W -Wall -Woverloaded-virtual dnl
-Wpointer-arith -Wcast-align -Wsign-compare -Wnon-virtual-dtor dnl
-Wno-long-long -Wundef -Wconversion -Wchar-subscripts -Wwrite-strings dnl
-Wmissing-format-attribute -Wcast-align -Wdisabled-optimization dnl
-Wnon-template-friend -Wsign-promo -Wno-ctor-dtor-privacy"
    fi

    if test x$debug = "xyes"; then
        M_CFLAGS="$M_CFLAGS -DG_DISABLE_DEPRECATED -DDEBUG -DENABLE_DEBUG -DENABLE_PROFILE -DG_ENABLE_DEBUG -DG_ENABLE_PROFILE"
        M_CXXFLAGS="$M_CXXFLAGS -DG_DISABLE_DEPRECATED -DDEBUG -DENABLE_DEBUG -DENABLE_PROFILE -DG_ENABLE_DEBUG -DG_ENABLE_PROFILE"
    else
        M_CFLAGS="$M_CFLAGS -DNDEBUG=1 -DG_DISABLE_CAST_CHECKS -DG_DISABLE_ASSERT"
        M_CXXFLAGS="$M_CXXFLAGS -DNDEBUG=1 -DG_DISABLE_CAST_CHECKS -DG_DISABLE_ASSERT"
    fi

    if test "x$all_gcc_warnings" = "xyes" -a "x$warnings_fatal" = "xyes"; then
        M_CXXFlAGS_NO_WERROR=$M_CXXFLAGS
        M_CXXFLAGS="-Werror $M_CXXFLAGS"
    fi

    AC_SUBST(M_CFLAGS)
    AC_SUBST(M_CXXFLAGS)
    AC_SUBST(M_CXXFlAGS_NO_WERROR)

    AC_MSG_CHECKING(for C compiler debug options)
    if test "x$M_CFLAGS" = "x"; then
        AC_MSG_RESULT(None)
    else
        AC_MSG_RESULT($M_CFLAGS)
    fi
    AC_MSG_CHECKING(for C++ compiler debug options)
    if test "x$M_CXXFLAGS" = "x"; then
        AC_MSG_RESULT(None)
    else
        AC_MSG_RESULT($M_CXXFLAGS)
    fi
])


AC_DEFUN([_CHECK_VERSION],[
    PKG_CHECK_MODULES($1, $2)

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

    AM_CONDITIONAL($1[]_2_6, test x$ver_2_6 = "xyes")
    AM_CONDITIONAL($1[]_2_4, test x$ver_2_4 = "xyes")
    AM_CONDITIONAL($1[]_2_2, test x$ver_2_2 = "xyes")

    AC_MSG_RESULT($[]$1[]_MAJOR_VERSION.$[]$1[]_MINOR_VERSION.$[]$1[]_MICRO_VERSION)
])

# PKG_CHECK_GTK_VERSIONS
AC_DEFUN([PKG_CHECK_GTK_VERSIONS],[
    _CHECK_VERSION(GTK, gtk+-2.0)
    _CHECK_VERSION(GLIB, glib-2.0)
    _CHECK_VERSION(GDK, gdk-2.0)
])# PKG_CHECK_GTK_VERSIONS
