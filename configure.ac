#
#   moo/configure.ac
#

m4_define([moo_major_version], [0])
m4_define([moo_minor_version], [5])
m4_define([moo_micro_version], [93])
m4_define([moo_version], [moo_major_version.moo_minor_version.moo_micro_version])

AC_INIT(libmoo, [moo_version], [muntyan@math.tamu.edu], libmoo)
AM_INIT_AUTOMAKE(libmoo, [moo_version])

AC_CONFIG_HEADERS(config.h)

AC_DEFINE(MOO_VERSION, ["moo_version"], "libmoo version")
MOO_VERSION=\"moo_version\"
MOO_VERSION_MAJOR=moo_major_version
MOO_VERSION_MINOR=moo_minor_version
MOO_VERSION_MICRO=moo_micro_version
AC_SUBST(MOO_VERSION)
AC_SUBST(MOO_VERSION_MAJOR)
AC_SUBST(MOO_VERSION_MINOR)
AC_SUBST(MOO_VERSION_MICRO)

AC_PROG_CC
AC_PROG_CC_STDC
AC_HEADER_STDC
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL
AC_PROG_INSTALL
AC_PROG_WINDRES
AC_PROG_YACC


# for pcre
AC_CHECK_FUNCS(memmove bcopy strerror)

#for GMappedFile
AC_CHECK_HEADERS(unistd.h)
AC_CHECK_FUNCS(mmap)


################################################################################
#  Terminal stuff
#
AC_CHECK_HEADERS(poll.h errno.h io.h fcntl.h sys/types.h sys/wait.h signal.h)
AC_CHECK_FUNCS([poll pipe _pipe kill signal posix_openpt])
AC_TYPE_SIGNAL
AC_TYPE_SIZE_T
AC_TYPE_PID_T

# Check for headers.
AC_CHECK_HEADERS(sys/select.h sys/termios.h sys/un.h stropts.h termios.h wchar.h)
AC_HEADER_TIOCGWINSZ

# Check for PTY handling functions.
AC_CHECK_FUNCS(cfmakeraw getpgid getpt grantpt unlockpt ptsname ptsname_r recvmsg)


################################################################################
#  Check debug options
#
# TODO TODO check gcc
AC_CHECK_DEBUG_STUFF([-Wall])


################################################################################
#  Check operating system
#
cygwin_build="no"
mingw_build="no"
case $host in
  *-*-mingw32*)
    echo "++++ building for MINGW32 ++++"
    mingw_build="yes"
    ;;
  *-*-cygwin*)
    echo "++++ building for CYGWIN ++++"
    cygwin_build="yes"
    ;;
  *-*-darwin*)
    echo "++++ building for DARWIN ++++"
    AC_DEFINE(OS_DARWIN, 1, [darwin])
    unix_build=yes
    ;;
  *)
    unix_build=yes
    ;;
esac
AM_CONDITIONAL(UNIX_BUILD, test x$unix_build = "xyes")
AM_CONDITIONAL(MINGW_BUILD, test x$mingw_build = "xyes")
AM_CONDITIONAL(CYGWIN_BUILD, test x$cygwin_build = "xyes")


################################################################################
#  GTK libraries
#
PKG_CHECK_GTK_VERSIONS
PKG_CHECK_MODULES(GLADE, [libglade-2.0])


################################################################################
#  File monitoring
#
if test x$unix_build = xyes; then
    AC_CHECK_FAM([has_fam=yes],[has_fam=no])
    if test x$has_fam = xyes; then
        AC_DEFINE(MOO_USE_FAM, 1, [use libfam for monitoring files])
    else
        AC_MSG_WARN([FAM or gamin not found. It is not required, everything will work but maybe not as well as with FAM.])
    fi
fi

# Win32 file watch needs a thread for WaitForMultipleObjects
if test x$mingw_build = xyes; then
    AC_DEFINE(MOO_NEED_THREAD, 1, [enable glib thread support])
    PKG_CHECK_MODULES(GTHREAD, [gthread-2.0])
fi


################################################################################
#  Python stuff
#
AC_ARG_WITH([pygtk],
  AC_HELP_STRING([--with-pygtk], [whether to compile pygtk support (default = autodetect)]),
  [
    if test x$with_pygtk = "xno"; then
        build_pygtk="no"
    else
        build_pygtk="yes"
    fi
  ], [
    build_pygtk="yes"
])


AC_ARG_WITH([python],
  AC_HELP_STRING([--with-python], [whether to compile python support (default = AUTO)]),
  [
    if test x$with_python = "xno"; then
        build_python="no"
        build_pygtk="no"
    else
        if test x$with_python = "xyes"; then
            python_version=2.2
        else
            python_version=$with_python
        fi
        build_python="yes"
    fi
  ], [
    if test x$build_pygtk = xyes; then
        build_python="yes"
    else
        build_python="auto"
        build_pygtk="no"
    fi
    python_version=2.2
])


AC_ARG_ENABLE([moo-module],
  AC_HELP_STRING([--enable-moo-module], [create standalone python module 'moo' (default = YES)]),
  [
    if test x$enable_moo_module = "xno"; then
        build_pymoo="no"
    else
        build_pymoo="yes"
    fi
  ], [
    build_pymoo="yes"
])


if test x$cygwin_build != "xyes" -a x$build_python != "xno"; then
    AC_CHECK_PYTHON($python_version,[
        build_python="yes"
        AC_MSG_NOTICE([compiling python support])
    ],[
        build_python="no"
        build_pygtk="no"
        if test x$build_python = "xyes"; then
            AC_MSG_ERROR([Python not found])
        else
            AC_MSG_WARN([Python support will be disabled])
        fi
    ])
fi

AM_CONDITIONAL(USE_PYTHON, test x$build_python = "xyes")
if test x$build_python = "xyes"; then
    AC_DEFINE(USE_PYTHON,, [USE_PYTHON])
fi


################################################################################
#  pygtk
#
if test x$cygwin_build != "xyes"; then
    if test x$build_pygtk = xyes; then
        AC_CHECK_PYGTK([$python_version],[
            build_pygtk="yes"
            AC_MSG_NOTICE([compiling pygtk support])
        ],[
            build_pygtk="no"
            AC_MSG_NOTICE([usable pygtk not found])
        ])
    fi
fi


AM_CONDITIONAL(USE_PYGTK, test x$build_pygtk = "xyes")
if test x$build_pygtk = "xyes"; then
    AC_DEFINE(USE_PYGTK,, [USE_PYGTK])
fi

if test x$build_pygtk = "xyes"; then
    if test x$build_pymoo != "xno"; then
        build_pymoo="yes"
    fi
else
    build_pymoo="no"
fi

AM_CONDITIONAL(BUILD_PYMOO, test x$build_pymoo = "xyes")


################################################################################
#  libxml2
#
AC_ARG_WITH([xml],
    AC_HELP_STRING([--with-xml], [enable xml]),
    [use_xml=$withval],
    [use_xml=auto]
)

if test x$cygwin_build != "xyes" -a x$use_xml != "xno"; then
    AC_CHECK_XML_STUFF([found_xml=yes],[found_xml=no])
fi

if test x$use_xml = "xyes" -a x$found_xml = "xno"; then
    AC_MSG_ERROR([libxml2 library not found])
fi
if test x$found_xml = "xyes"; then
    use_xml=yes
    AC_MSG_NOTICE([compiling with xml support])
    AC_DEFINE(MOO_USE_XML,, [use libxml])
else
    use_xml=no
    AC_MSG_NOTICE([compiling without xml support])
fi

AM_CONDITIONAL(MOO_USE_XML, test x$use_xml = xyes)


################################################################################
#  xdgmime
#
if test x$unix_build = "xyes"; then
    AC_DEFINE(MOO_USE_XDGMIME,, [use xdgmime])
fi
AM_CONDITIONAL(MOO_USE_XDGMIME, test x$unix_build = "xyes")


################################################################################
#  Components
#
AC_ARG_WITH([mooapp],
    AC_HELP_STRING([--without-mooapp], [disable building mooapp]),
    [build_mooapp=$withval],
    [build_mooapp=yes]
)
AC_ARG_WITH([mooedit],
    AC_HELP_STRING([--without-mooedit], [disable building mooedit]),
    [build_mooedit=$withval],
    [build_mooedit=yes]
)
AC_ARG_WITH([mooutils],
    AC_HELP_STRING([--without-mooutils], [disable building mooutils]),
    [build_mooutils=$withval],
    [build_mooutils=yes]
)
AC_ARG_WITH([mooui],
    AC_HELP_STRING([--without-mooui], [disable building mooui]),
    [build_mooui=$withval],
    [build_mooui=yes]
)
AC_ARG_WITH([mooterm],
    AC_HELP_STRING([--without-mooterm], [disable building mooterm]),
    [build_mooterm=$withval],
    [build_mooterm=yes]
)


if test x$cygwin_build = "xyes"; then
    build_mooutils="no"
    build_mooui="no"
    build_mooedit="no"
    build_mooapp="no"
fi

if test "x$build_mooapp" != "xno"; then
    build_mooedit="yes"
    build_mooterm="yes"
fi
if test "x$build_mooedit" != "xno"; then
    build_mooui="yes"
fi
if test "x$build_mooterm" != "xno" -a x$cygwin_build != "xyes"; then
    build_mooutils="yes"
fi
if test "x$build_mooui" != "xno"; then
    build_mooutils="yes"
fi


AM_CONDITIONAL(BUILD_MOOUTILS,  test "x$build_mooutils"  != "xno")
AM_CONDITIONAL(BUILD_MOOUI,     test "x$build_mooui"     != "xno")
AM_CONDITIONAL(BUILD_MOOEDIT,   test "x$build_mooedit"   != "xno")
AM_CONDITIONAL(BUILD_MOOTERM,   test "x$build_mooterm"   != "xno")
AM_CONDITIONAL(BUILD_MOOAPP,    test "x$build_mooapp"    != "xno")


moo_top_src_dir=`cd $srcdir && pwd`
moo_top_build_dir=`cd ./$ac_top_builddir && pwd`
MOO_CFLAGS="-I$moo_top_src_dir/moo $GTK_CFLAGS"
MOO_LIBS="$moo_top_build_dir/moo/libmoo.la $GTK_LIBS $GLADE_LIBS"

if test x$has_fam = xyes; then
    MOO_LIBS="$MOO_LIBS $FAM_LIBS"
fi

AC_SUBST(MOO_CFLAGS)
AC_SUBST(MOO_LIBS)


################################################################################
#  MooEdit stuff
#
if test "x$build_mooedit" != "xno"; then
    MOO_CFLAGS="$MOO_CFLAGS $XML_CFLAGS -I$moo_top_src_dir/moo/mooedit"
    MOO_LIBS="$MOO_LIBS $XML_LIBS"
fi


################################################################################
#  Installation directories, etc.
#
MOO_SRC_PREFIX="moo"
AC_SUBST(MOO_SRC_PREFIX)

if test "x${prefix}" = "xNONE"; then
  mooprefix=${ac_default_prefix}
else
  mooprefix=${prefix}
fi

moodatadir=share

moorootdir=${moodatadir}/moo
NO_PREFIX_MOO_ROOT_DIR="${moorootdir}"
AC_SUBST(NO_PREFIX_MOO_ROOT_DIR)
MOO_ROOT_DIR="${mooprefix}/${moorootdir}"
AC_SUBST(MOO_ROOT_DIR)

AC_DEFINE_UNQUOTED(MOO_ROOT_DIR, "${mooprefix}/${moorootdir}", [data files go to MOO_ROOT_DIR])

MOO_EDIT_LANG_FILES_DIR="${mooprefix}/${moorootdir}/language-specs"
AC_SUBST(MOO_EDIT_LANG_FILES_DIR)
AC_DEFINE_UNQUOTED(MOO_EDIT_LANG_FILES_DIR, "${mooprefix}/${moorootdir}/language-specs", [lang files dir])


moolibdir=lib/moo
NO_PREFIX_MOO_LIB_DIR="${moolibdir}"
AC_SUBST(NO_PREFIX_MOO_LIB_DIR)
MOO_LIB_DIR="${mooprefix}/${moolibdir}"
AC_SUBST(MOO_LIB_DIR)
AC_DEFINE_UNQUOTED(MOO_LIB_DIR, "${mooprefix}/${moolibdir}", [plugins and stuff go to MOO_LIB_DIR])
MOO_EDIT_PLUGINS_DIR="${mooprefix}/${moolibdir}/plugins"
AC_SUBST(MOO_EDIT_PLUGINS_DIR)
AC_DEFINE_UNQUOTED(MOO_EDIT_PLUGINS_DIR, "${mooprefix}/${moolibdir}/plugins", [plugins dir])


################################################################################
#  Created files
#
AC_OUTPUT([
Makefile
moo/Makefile
moo/mooapp/Makefile
moo/mooapp/glade/Makefile
moo/mooedit/Makefile
moo/mooedit/gtksourceview/Makefile
moo/mooedit/xdgmime/Makefile
moo/mooedit/glade/Makefile
moo/mooedit/language-specs/Makefile
moo/mooedit/moofileview/Makefile
moo/mooedit/moofileview/glade/Makefile
moo/mooedit/plugins/Makefile
moo/mooedit/plugins/cproject/Makefile
moo/mooterm/Makefile
moo/mooterm/termhelper_res.rc
moo/mooterm/glade/Makefile
moo/mooui/Makefile
moo/mooui/glade/Makefile
moo/mooutils/Makefile
moo/mooutils/glade/Makefile
moo/mooutils/newgtk/Makefile
moo/mooutils/pcre/Makefile
moo/mooutils/pixmaps/Makefile
moo/mooutils/unrtf/Makefile
moo/moopython/Makefile
tests/Makefile
tests/medit.c
tests/pyapp.py
])


################################################################################
#  Configuration info
#
if test x$cygwin_build != "xyes"; then
    if test "x$mingw_build" = "xyes"; then
        system="MINGW"
    elif test "x$darwin_build" = "xyes"; then
        system="DARWIN"
    else
        system="UNIX"
    fi

    if test x$use_xml != "xyes"; then
        use_xml="no"
    fi

    echo
    echo "    OS                                        = $system"
    echo "    prefix                                    = $mooprefix"
    echo "    python support                            = $build_python"
    echo "    pygtk support                             = $build_pygtk"
    echo "    standalone 'moo' python module            = $build_pymoo"
    echo "    xml support                               = $use_xml"
    echo
    echo "    editor lang files go to                   = $MOO_EDIT_LANG_FILES_DIR"
    echo
else
    echo
    echo "    Building termhelper.exe on CYGWIN"
    echo
fi
