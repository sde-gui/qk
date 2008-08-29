#
#   moo/configure.ac
#

# keep version in sync with doc/medit.t2t
AC_INIT([medit],[0.9.5],[muntyan@tamu.edu],[medit])
AM_INIT_AUTOMAKE([foreign])
AC_CANONICAL_HOST

AC_CONFIG_HEADERS(config.h)
MOO_DEFINE_VERSIONS(moo, AC_PACKAGE_VERSION)
MOO_DEFINE_MODULE_VERSIONS(1,0)

MOO_AC_I18N(moo)
MOO_AC_PO_GSV

AC_PROG_CC

# m4_define(MOO_USE_CPP_DEFINE, 1)
m4_ifdef([MOO_USE_CPP_DEFINE],[AC_PROG_CXX],[m4_define([_LT_AC_TAGCONFIG],[])])

AC_DISABLE_STATIC
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL
AC_PROG_INSTALL

ACLOCAL="$ACLOCAL $ACLOCAL_FLAGS"

MOO_COMPONENTS([app,edit,term],[])
MOO_AC_SET_DIRS
MOO_AC_PRIV_FLAGS
MOO_AM_MK

# AM_CONDITIONAL([MOO_USE_CPP],[test x$MOO_ENABLE_PROJECT = xyes])


################################################################################
#  Created files
#
AC_CONFIG_FILES([
Makefile
doc/Makefile
moo/Makefile
moo/mooapp/Makefile
moo/mooedit/Makefile
moo/moofileview/Makefile
moo/moolua/Makefile
moo/moopython/Makefile
moo/moopython/plugins/Makefile
moo/moopython/plugins/pyproject/Makefile
moo/mooutils/Makefile
moo/mooutils/newgtk/Makefile
moo/mooutils/pcre/Makefile
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
  echo
  echo "    components: $MOO_BUILD_COMPS"
  echo
else
  echo
  echo "    Building termhelper.exe on CYGWIN"
  echo
fi
