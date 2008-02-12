#
#   moo/configure.ac
#

AC_INIT([medit],[0.9.4],[muntyan@tamu.edu],[medit])
AM_INIT_AUTOMAKE
AC_CANONICAL_HOST

AC_CONFIG_HEADERS(config.h)
MOO_DEFINE_VERSIONS(moo, AC_PACKAGE_VERSION)
MOO_DEFINE_MODULE_VERSIONS(1,0)

MOO_AC_I18N(moo)
MOO_AC_PO_GSV

AC_PROG_CC
m4_define([_LT_AC_TAGCONFIG],[])
AC_DISABLE_STATIC
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL
AC_PROG_INSTALL

AC_PROG_OBJC
MOO_AC_OBJC

MOO_AC_PROGS

ACLOCAL="$ACLOCAL $ACLOCAL_FLAGS"

if test "x$MOO_USE_PIPE_INPUT" = xyes; then
  AC_DEFINE([MOO_USE_PIPE_INPUT], [1], [use pipes for input])
fi

MOO_SRC_PREFIX="moo"
AC_SUBST(MOO_SRC_PREFIX)
MOO_COMPONENTS([app,edit,term],[])
MOO_AC_SET_DIRS
MOO_AC_FLAGS(moo)


################################################################################
#  Created files
#
AC_CONFIG_FILES([
Makefile
config.py
doc/Makefile
moo/moo.pc
moo/moo-config.h
moo/Makefile
moo/moolua/Makefile
moo/mooutils/Makefile
moo/mooutils/newgtk/Makefile
moo/moofileview/Makefile
moo/mooterm/Makefile
moo/mooedit/Makefile
moo/mooedit/language-specs/Makefile
moo/mooedit/gtksourceview/Makefile
moo/mooedit/gtksourceview/upstream/Makefile
moo/mooedit/plugins/Makefile
moo/mooedit/plugins/ctags/Makefile
moo/mooapp/Makefile
moo/mooapp/smclient/Makefile
moo/moopython/Makefile
moo/moopython/plugins/Makefile
moo/moopython/plugins/pyproject/Makefile
moo/moopython/pygtk/Makefile
moo/mooutils/pcre/Makefile
moo/mooutils/xdgmime/Makefile
moo/mooterm/mootermhelper_res.rc
tests/pyapp.py
tests/Makefile
medit/Makefile
medit/medit.iss
po/Makefile.in
po-gsv/Makefile.in
])
AC_OUTPUT

################################################################################
#  Configuration info
#
if test "x$MOO_USE_PYTHON" = "xyes" -o "x$MOO_USE_PYTHON" = "xtrue"; then
  use_python=yes
else
  use_python=no
fi

if test x$MOO_OS_CYGWIN != "xyes"; then
  echo
  echo "    OS ...................................... $MOO_OS_NAME"
  echo "    use python .............................. $use_python"
  echo "    use xml ................................. $MOO_USE_XML"
  echo "    build pcre library ...................... $MOO_BUILD_PCRE"
  echo "    use FAM ................................. $MOO_USE_FAM"
  echo
  echo "    prefix .................................. $prefix"
  echo "    editor lang files go to ................. $MOO_TEXT_LANG_FILES_DIR"
  echo "    build html help ......................... $enable_help"
  echo
  echo "    components: $MOO_BUILD_COMPS"
  echo
else
  echo
  echo "    Building termhelper.exe on CYGWIN"
  echo
fi
