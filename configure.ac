#
#   moo/configure.ac
#

AC_INIT([medit],[0.8.4],[muntyan@tamu.edu],[medit])
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AC_CANONICAL_HOST

AC_CONFIG_HEADERS(config.h)
MOO_DEFINE_VERSIONS(moo, AC_PACKAGE_VERSION)
MOO_DEFINE_MODULE_VERSIONS(1,0)

MOO_AC_I18N(moo)

AC_PROG_CC
AC_DISABLE_STATIC
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL
AC_PROG_INSTALL
MOO_AC_PROGS

ACLOCAL="$ACLOCAL $ACLOCAL_FLAGS"

MOO_SRC_PREFIX="moo"
AC_SUBST(MOO_SRC_PREFIX)
MOO_COMPONENTS([app,edit,term],[])
MOO_AC_SET_DIRS(moo)
MOO_AC_FLAGS(moo)


################################################################################
#  Created files
#
AC_CONFIG_FILES([
Makefile
config.py
moo/moo.pc
moo/moo-config.h
moo/Makefile
moo/mooutils/Makefile
moo/mooscript/Makefile
moo/moofileview/Makefile
moo/mooterm/Makefile
moo/mooedit/Makefile
moo/mooedit/language-specs/Makefile
moo/mooedit/gtksourceview/Makefile
moo/mooedit/gtksourceview/upstream/Makefile
moo/mooedit/plugins/Makefile
moo/mooedit/plugins/activestrings/Makefile
moo/mooedit/plugins/fileselector/Makefile
moo/mooapp/Makefile
moo/mooapp/smclient/Makefile
moo/moopython/Makefile
moo/moopython/plugins/Makefile
moo/moopython/plugins/pyproject/Makefile
moo/moopython/pygtk/Makefile
moo/mooutils/pcre/Makefile
moo/mooutils/xdgmime/Makefile
moo/mooterm/termhelper_res.rc
tests/pyapp.py
tests/Makefile
medit/Makefile
medit/medit.iss
po/Makefile.in
])
AC_OUTPUT


################################################################################
#  Configuration info
#
if test x$MOO_OS_CYGWIN != "xyes"; then
  echo
  echo "    OS ...................................... $MOO_OS_NAME"
  echo "    use python .............................. $MOO_USE_PYTHON"
  echo "    use xml ................................. $MOO_USE_XML"
  echo "    use xdgmime ............................. $MOO_USE_XDGMIME"
  echo "    build pcre library ...................... $MOO_BUILD_PCRE"
  echo "    use FAM ................................. $MOO_USE_FAM"
  echo
  echo "    prefix .................................. $prefix"
  echo "    editor lang files go to ................. $MOO_TEXT_LANG_FILES_DIR"
  echo
  echo "    components: $MOO_BUILD_COMPS"
  echo
else
  echo
  echo "    Building termhelper.exe on CYGWIN"
  echo
fi
