#
#   moo/configure.ac
#

m4_define([moo_major_version], [0])
m4_define([moo_minor_version], [8])
m4_define([moo_micro_version], [0])
m4_define([moo_version], [moo_major_version.moo_minor_version.moo_micro_version])

AC_INIT(medit, [moo_version], [muntyan@tamu.edu], medit)
AM_INIT_AUTOMAKE(medit, [moo_version])
AC_CANONICAL_HOST

AC_CONFIG_HEADERS(config.h)
MOO_DEFINE_VERSIONS

MOO_AC_I18N(moo, [])

#  Check operating system
MOO_AC_CHECK_OS

if test x$MOO_OS_UNIX != xyes; then
AM_CONDITIONAL(MOO_INSTALL_LIB, true)
AM_CONDITIONAL(MOO_BUILD_PYTHON_MODULE, true)
AM_CONDITIONAL(MOO_INSTALL_HEADERS, false)
AC_DEFINE(MOO_BUILD_PYTHON_MODULE, 1, MOO_BUILD_PYTHON_MODULE)
else
AM_CONDITIONAL(MOO_INSTALL_LIB, true)
AM_CONDITIONAL(MOO_BUILD_PYTHON_MODULE, false)
AM_CONDITIONAL(MOO_INSTALL_HEADERS, false)
fi

AC_PROG_CC
AC_PROG_CC_STDC
AC_DISABLE_STATIC
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL
AC_PROG_INSTALL
MOO_AC_PROGS

ACLOCAL="$ACLOCAL $ACLOCAL_FLAGS"

#  Misc stuff
MOO_AC_FUNCS

#  Check debug options
MOO_AC_DEBUG

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
MOO_COMPONENTS
MOO_AC_RELOC
MOO_AC_SET_DIRS(moo)
MOO_AC_FLAGS(moo)


################################################################################
#  Created files
#
AC_OUTPUT([
Makefile
moo/Makefile
moo/mooutils/Makefile
moo/mooscript/Makefile
moo/moofileview/Makefile
moo/mooterm/Makefile
moo/mooedit/Makefile
moo/mooedit/language-specs/Makefile
moo/mooedit/gtksourceview/Makefile
moo/mooedit/plugins/Makefile
moo/mooedit/plugins/activestrings/Makefile
moo/mooedit/plugins/completion/Makefile
moo/mooedit/plugins/fileselector/Makefile
moo/mooapp/Makefile
moo/moopython/Makefile
moo/moopython/plugins/Makefile
moo/moopython/plugins/pyproject/Makefile
moo/moopython/pygtk/Makefile
moo/mooutils/pcre/Makefile
moo/mooutils/xdgmime/Makefile
m4/Makefile
tests/Makefile
po/Makefile.in
moo/mooterm/termhelper_res.rc
moo/moo.pc
moo/moo-version.h
tests/pyapp.py
tests/medit.rc
wininstaller/medit.iss
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
    echo "    xdgmime support                           = $MOO_USE_XDGMIME"
    echo "    build pcre library                        = $MOO_BUILD_PCRE"
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
