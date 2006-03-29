#
#   moo/configure.ac
#

m4_define([moo_major_version], [0])
m4_define([moo_minor_version], [6])
m4_define([moo_micro_version], [6])
m4_define([moo_version], [moo_major_version.moo_minor_version.moo_micro_version])

AC_INIT(libmoo, [moo_version], [muntyan@math.tamu.edu], libmoo)
AM_INIT_AUTOMAKE(libmoo, [moo_version])
AC_CANONICAL_HOST

AC_CONFIG_HEADERS(config.h)
MOO_DEFINE_VERSIONS

AC_PROG_CC
AC_PROG_CC_STDC
AC_HEADER_STDC
# AC_LIBTOOL_WIN32_DLL
# AC_PROG_LIBTOOL
AC_PROG_INSTALL
MOO_AC_PROG_WINDRES

AC_SUBST(ACLOCAL_FLAGS)

#  Misc stuff
MOO_AC_FUNCS

#  Check debug options
MOO_AC_DEBUG

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

# pcre
MOO_AC_PCRE


MOO_SRC_PREFIX="moo"
AC_SUBST(MOO_SRC_PREFIX)
AM_CONDITIONAL(MOO_BUILD_LIB, false)
MOO_COMPONENTS
MOO_AC_SET_DIRS([moo-1.0])
MOO_AC_FLAGS


################################################################################
#  Created files
#
AC_OUTPUT([
Makefile
moo/mooterm/termhelper_res.rc
moo/mooutils/pcre/pcre.h
moo/moo.pc
tests/pyapp.py
m4/Makefile
])


################################################################################
#  Configuration info
#
if test x$MOO_OS_CYGWIN != "xyes"; then
    if test "x$MOO_OS_MINGW" = "xyes"; then
        system="MINGW"
    elif test "x$MOO_OS_DARWIN" = "xyes"; then
        system="UNIX (DARWIN)"
    elif test "x$MOO_OS_BSD" = "xyes"; then
        system="UNIX (BSD)"
    else
        system="UNIX"
    fi

    if test x$use_xml != "xyes"; then
        use_xml="no"
    fi

    echo
    echo "    OS                                        = $system"
    echo "    python support                            = $MOO_USE_PYTHON"
    echo "    pygtk support                             = $MOO_USE_PYGTK"
    echo "    xml support                               = $MOO_USE_XML"
    echo "    fam support                               = $MOO_USE_FAM"
    echo "    xdgmime support                           = $MOO_USE_XDGMIME"
    echo
    echo "    prefix                                    = $prefix"
    echo "    editor lang files go to                   = $MOO_TEXT_LANG_FILES_DIR"
    echo
    echo "    components: $MOO_BUILD_COMPS"
else
    echo
    echo "    Building termhelper.exe on CYGWIN"
    echo
fi
