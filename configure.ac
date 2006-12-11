#
#   moo/configure.ac
#

m4_define([moo_major_version], [0])
m4_define([moo_minor_version], [8])
m4_define([moo_micro_version], [1])
m4_define([moo_version], [moo_major_version.moo_minor_version.moo_micro_version])

m4_define([moo_module_major_version], [1])
m4_define([moo_module_minor_version], [0])

AC_INIT([medit],[moo_version],[muntyan@tamu.edu],[medit])
AM_INIT_AUTOMAKE(medit, [moo_version])
AC_CANONICAL_HOST

AC_CONFIG_HEADERS(config.h)
MOO_DEFINE_VERSIONS

MOO_AC_I18N(moo, [ru])

AC_PROG_CC
AC_DISABLE_SHARED
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL
AC_PROG_INSTALL
MOO_AC_PROGS

ACLOCAL="$ACLOCAL $ACLOCAL_FLAGS"

MOO_SRC_PREFIX="moo"
AC_SUBST(MOO_SRC_PREFIX)
MOO_COMPONENTS(term)
MOO_AC_SET_DIRS(moo)
MOO_AC_FLAGS(moo)


################################################################################
#  Created files
#
AC_CONFIG_FILES([
Makefile
config.py
moo/moo.pc
moo/moo-version.h
moo/libmoo.rc
moo/moo24.rc
moo/moo25.rc
moo/pymoo24.rc
moo/pymoo25.rc
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
moo/moopython/pymoo.ini
moo/moopython/pymoo24.ini
moo/moopython/pymoo25.ini
moo/moopython/plugins/Makefile
moo/moopython/plugins/python.ini.desktop.in
moo/moopython/plugins/terminal.ini.desktop.in
moo/moopython/plugins/pyproject/Makefile
moo/moopython/plugins/pyproject/project-plugin.ini.desktop.in
moo/moopython/pygtk/Makefile
moo/mooutils/pcre/Makefile
moo/mooutils/xdgmime/Makefile
moo/mooterm/termhelper_res.rc
tests/pyapp.py
tests/medit.rc
tests/Makefile
po/Makefile.in
wininstaller/medit.iss
])
AC_OUTPUT


################################################################################
#  Configuration info
#
echo
if test x$MOO_OS_CYGWIN != "xyes"; then
  echo "    OS                                        = $MOO_OS_NAME"
  echo "    python support                            = $MOO_USE_PYTHON"
  echo "    pygtk support                             = $MOO_USE_PYGTK"
  echo "    xml support                               = $MOO_USE_XML"
  echo "    xdgmime support                           = $MOO_USE_XDGMIME"
  echo "    build pcre library                        = $MOO_BUILD_PCRE"
  echo "    use FAM                                   = $MOO_USE_FAM"
  echo
  echo "    prefix                                    = $prefix"
  echo "    editor lang files go to                   = $MOO_TEXT_LANG_FILES_DIR"
  echo
  echo "    components: $MOO_BUILD_COMPS"
else
  echo "    Building termhelper.exe on CYGWIN"
fi
echo
