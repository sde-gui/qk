#
#   moo/configure.ac
#

m4_define([moo_major_version], [0])
m4_define([moo_minor_version], [6])
m4_define([moo_micro_version], [0])
m4_define([moo_api_version], [1.0])
m4_define([moo_version], [moo_major_version.moo_minor_version.moo_micro_version])

AC_INIT(libmoo, [moo_version], [muntyan@math.tamu.edu], libmoo)
AM_INIT_AUTOMAKE(libmoo, [moo_version])

AC_CONFIG_HEADERS(config.h)

MOO_VERSION=\"moo_version\"
MOO_VERSION_UNQUOTED=moo_version
MOO_API_VERSION=moo_api_version
MOO_VERSION_MAJOR=moo_major_version
MOO_VERSION_MINOR=moo_minor_version
MOO_VERSION_MICRO=moo_micro_version
AC_SUBST(MOO_VERSION)
AC_SUBST(MOO_VERSION_UNQUOTED)
AC_SUBST(MOO_API_VERSION)
AC_SUBST(MOO_VERSION_MAJOR)
AC_SUBST(MOO_VERSION_MINOR)
AC_SUBST(MOO_VERSION_MICRO)

AC_DEFINE(MOO_VERSION, ["moo_version"], "libmoo version")
AC_DEFINE(MOO_API_VERSION, ["moo_api_version"], "libmoo api version")
AC_DEFINE(MOO_VERSION_MAJOR, [moo_major_version], "libmoo major version")
AC_DEFINE(MOO_VERSION_MINOR, [moo_minor_version], "libmoo minor version")
AC_DEFINE(MOO_VERSION_MICRO, [moo_micro_version], "libmoo micro version")

AC_PROG_CC
AC_PROG_CC_STDC
AC_HEADER_STDC
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL
AC_PROG_INSTALL
MOO_AC_PROG_WINDRES


# for pcre
AC_CHECK_FUNCS(memmove bcopy strerror)

#for GMappedFile
AC_CHECK_HEADERS(unistd.h)
AC_CHECK_FUNCS(mmap)

#for moofileutils.c
AC_CHECK_FUNCS(unlink)


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
#
################################################################################


#  Check debug options
# TODO TODO check gcc
MOO_AC_CHECK_DEBUG_STUFF([-Wall])

#  Check operating system
MOO_AC_CHECK_OS

#  GTK libraries
MOO_PKG_CHECK_GTK_VERSIONS

#  File monitoring
MOO_AC_FAM

#  libxml2
MOO_AC_XML

#  xdgmime
MOO_AC_XDGMIME

#  Python stuff
MOO_AC_PYGTK


################################################################################
#  Python module
#
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

if test x$MOO_USE_PYGTK = "xyes"; then
    if test x$build_pymoo != "xno"; then
        build_pymoo="yes"
    fi
else
    build_pymoo="no"
fi

AM_CONDITIONAL(BUILD_PYMOO, test x$build_pymoo = "xyes")


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
AC_ARG_WITH([mooterm],
    AC_HELP_STRING([--without-mooterm], [disable building mooterm]),
    [build_mooterm=$withval],
    [build_mooterm=yes]
)


if test x$MOO_OS_CYGWIN = "xyes"; then
    build_mooutils="no"
    build_mooedit="no"
    build_mooapp="no"
fi

if test "x$build_mooapp" != "xno"; then
    build_mooedit="yes"
fi
if test "x$build_mooedit" != "xno"; then
    build_mooutils="yes"
fi
if test "x$build_mooterm" != "xno" -a x$MOO_OS_CYGWIN != "xyes"; then
    build_mooutils="yes"
fi


AM_CONDITIONAL(MOO_BUILD_UTILS,     test "x$build_mooutils"  != "xno")
AM_CONDITIONAL(MOO_BUILD_EDIT,      test "x$build_mooedit"   != "xno")
AM_CONDITIONAL(MOO_BUILD_TERM,      test "x$build_mooterm"   != "xno")
AM_CONDITIONAL(MOO_BUILD_APP,       test "x$build_mooapp"    != "xno")
AM_CONDITIONAL(MOO_BUILD_PLUGINS,   true)

if test "x$build_mooutils" != "xno"; then
    AC_DEFINE(MOO_BUILD_UTILS,, [build mooutils])
fi
if test "x$build_mooedit" != "xno"; then
    AC_DEFINE(MOO_BUILD_EDIT,, [build mooedit])
fi
if test "x$build_mooterm" != "xno"; then
    AC_DEFINE(MOO_BUILD_TERM,, [build mooterm])
fi
if test "x$build_mooapp" != "xno"; then
    AC_DEFINE(MOO_BUILD_APP,, [build mooapp])
fi

moo_top_src_dir=`cd $srcdir && pwd`
moo_top_build_dir=`cd ./$ac_top_builddir && pwd`
MOO_CFLAGS="-I$moo_top_src_dir/moo $GTK_CFLAGS -DXDG_PREFIX=_moo_edit_xdg -DG_LOG_DOMAIN=\\\"Moo\\\" -D__MOO__ -DMOO_MARSHALS_H=\\\"mooutils/moomarshals.h\\\""
MOO_LIBS="$GTK_LIBS"

if test x$MOO_USE_FAM = xyes; then
    MOO_LIBS="$MOO_LIBS $FAM_LIBS"
fi

AC_SUBST(MOO_CFLAGS)
AC_SUBST(MOO_LIBS)


################################################################################
#  MooEdit stuff
#
if test "x$build_mooedit" != "xno"; then
    MOO_CFLAGS="$MOO_CFLAGS $XML_CFLAGS"
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

moorootdir=${moodatadir}/moo-1.0
NO_PREFIX_MOO_ROOT_DIR="${moorootdir}"
AC_SUBST(NO_PREFIX_MOO_ROOT_DIR)
MOO_ROOT_DIR="${mooprefix}/${moorootdir}"
AC_SUBST(MOO_ROOT_DIR)

AC_DEFINE_UNQUOTED(MOO_ROOT_DIR, "${mooprefix}/${moorootdir}", [data files go to MOO_ROOT_DIR])

MOO_TEXT_LANG_FILES_DIR="${mooprefix}/${moorootdir}/syntax"
AC_SUBST(MOO_TEXT_LANG_FILES_DIR)
AC_DEFINE_UNQUOTED(MOO_TEXT_LANG_FILES_DIR, "${mooprefix}/${moorootdir}/syntax", [lang files dir])


moolibdir=lib/moo-1.0
NO_PREFIX_MOO_LIB_DIR="${moolibdir}"
AC_SUBST(NO_PREFIX_MOO_LIB_DIR)
MOO_LIB_DIR="${mooprefix}/${moolibdir}"
AC_SUBST(MOO_LIB_DIR)
AC_DEFINE_UNQUOTED(MOO_LIB_DIR, "${mooprefix}/${moolibdir}", [plugins and stuff go to MOO_LIB_DIR])
MOO_PLUGINS_DIR="${mooprefix}/${moolibdir}/plugins"
AC_SUBST(MOO_PLUGINS_DIR)
AC_DEFINE_UNQUOTED(MOO_PLUGINS_DIR, "${mooprefix}/${moolibdir}/plugins", [plugins dir])


AM_CONDITIONAL(MOO_STANDALONE, true)


################################################################################
#  Created files
#
AC_OUTPUT([
Makefile
moo/moo-config.h
moo/Makefile
moo/mooapp/Makefile
moo/mooapp/glade/Makefile
moo/mooedit/Makefile
moo/mooedit/glade/Makefile
moo/mooedit/syntax/Makefile
moo/mooedit/plugins/Makefile
moo/mooterm/Makefile
moo/mooterm/termhelper_res.rc
moo/mooterm/glade/Makefile
moo/mooutils/Makefile
moo/mooutils/glade/Makefile
moo/mooutils/newgtk/Makefile
moo/mooutils/pcre/Makefile
moo/mooutils/pixmaps/Makefile
moo/mooutils/moofileview/Makefile
moo/mooutils/moofileview/glade/Makefile
moo/mooutils/xdgmime/Makefile
moo/moopython/Makefile
tests/Makefile
tests/pyapp.py
m4/Makefile
moo.pc
])


################################################################################
#  Configuration info
#
if test x$MOO_OS_CYGWIN != "xyes"; then
    if test "x$MOO_OS_MINGW" = "xyes"; then
        system="MINGW"
    elif test "x$MOO_OS_DARWIN" = "xyes"; then
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
    echo "    editor lang files go to                   = $MOO_TEXT_LANG_FILES_DIR"
    echo
else
    echo
    echo "    Building termhelper.exe on CYGWIN"
    echo
fi
