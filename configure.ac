m4_define([_moo_major_version_],[0])
m4_define([_moo_minor_version_],[99])
m4_define([_moo_micro_version_],[3])
m4_define([_moo_version_suffix_],[devel])
m4_define([_moo_module_major_version_],[2])
m4_define([_moo_module_minor_version_],[0])
m4_define([_moo_version_],[_moo_major_version_._moo_minor_version_._moo_micro_version_])

m4_if([_moo_version_suffix_],[],[m4_define([_moo_display_version_],[_moo_version_])],[m4_define([_moo_display_version_],[_moo_version_-_moo_version_suffix_])])

m4_define([_moo_website_],[http://mooedit.sourceforge.net/])
m4_define([_moo_web_contact_],[http://mooedit.sourceforge.net/contact.html])
m4_define([_moo_email_],[emuntyan@sourceforge.net])
m4_define([_moo_copyright_],[2004-2010 Yevgen Muntyan <_moo_email_>])

AC_PREREQ([2.65])
AC_INIT([medit], [_moo_display_version_], [_moo_email_])
AC_GNU_SOURCE
AC_CONFIG_MACRO_DIR([m4])
AC_SUBST(ACLOCAL_FLAGS)

AM_INIT_AUTOMAKE([1.11 silent-rules foreign dist-bzip2 no-dist-gzip])
# LT_INIT([disable-shared])

AC_SUBST(MOO_MAJOR_VERSION,_moo_major_version_)
AC_SUBST(MOO_MINOR_VERSION,_moo_minor_version_)
AC_SUBST(MOO_MICRO_VERSION,_moo_micro_version_)
AC_SUBST(MOO_MODULE_MAJOR_VERSION,_moo_module_major_version_)
AC_SUBST(MOO_MODULE_MINOR_VERSION,_moo_module_minor_version_)
AC_SUBST(MOO_VERSION,_moo_version_)
AC_SUBST(MOO_DISPLAY_VERSION,"_moo_display_version_")
AC_SUBST(MOO_EMAIL,"_moo_email_")
AC_SUBST(MOO_WEBSITE,"_moo_website_")
AC_SUBST(MOO_WEB_CONTACT,"_moo_web_contact_")
AC_SUBST(MOO_COPYRIGHT,"_moo_copyright_")

# keep in sync with po/maintain
AC_SUBST(MOO_PACKAGE_NAME,"medit-1")
MOO_INI_FILE_NAME="medit.ini"
MOO_STATE_FILE_NAME="medit.state"
MOO_SESSION_FILE_NAME="medit.session"

AC_SUBST(GETTEXT_PACKAGE,$MOO_PACKAGE_NAME)
AC_SUBST(GETTEXT_PACKAGE_GSV,"$MOO_PACKAGE_NAME-gsv")
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE",[GETTEXT_PACKAGE])
MOO_INTL

AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_MKDIR_P
AM_PROG_CC_C_O

AC_ARG_VAR([WINDRES], [windres])
AC_CHECK_TOOL(WINDRES, windres, :)

AC_ARG_VAR([PYTHON], [Python executable, used to run build scripts])
AC_CHECK_PROGS(PYTHON, python, [AC_MSG_ERROR([Python not found])])
AC_ARG_VAR([PERL], [Perl executable, used to run build scripts])
AC_CHECK_PROGS(PERL, perl, [AC_MSG_ERROR([Perl not found])])

MOO_AC_FLAGS

AC_CONFIG_HEADERS([config.h])
AH_TOP([#ifndef __CONFIG_H__
#define __CONFIG_H__])
AH_BOTTOM([#include <moo-config.h>
#endif // __CONFIG_H__])

MOO_LT_LIB_M

AC_DEFINE(MOO_BUILD_APP,[1],[MOO_BUILD_APP])
AC_DEFINE(MOO_BUILD_EDIT,[1],[MOO_BUILD_EDIT])
AC_DEFINE(MOO_BUILD_LUA,[1],[MOO_BUILD_LUA])
AC_DEFINE(MOO_BUILD_UTILS,[1],[MOO_BUILD_UTILS])

AC_DEFINE_UNQUOTED(MOO_VERSION,["$MOO_VERSION"],[MOO_VERSION])
AC_DEFINE_UNQUOTED(MOO_DISPLAY_VERSION,["$MOO_DISPLAY_VERSION"],[MOO_DISPLAY_VERSION])
AC_DEFINE_UNQUOTED(MOO_MAJOR_VERSION,[$MOO_MAJOR_VERSION],[MOO_MAJOR_VERSION])
AC_DEFINE_UNQUOTED(MOO_MICRO_VERSION,[$MOO_MICRO_VERSION],[MOO_MICRO_VERSION])
AC_DEFINE_UNQUOTED(MOO_MINOR_VERSION,[$MOO_MINOR_VERSION],[MOO_MINOR_VERSION])
AC_DEFINE_UNQUOTED(MOO_MODULE_MAJOR_VERSION,[$MOO_MODULE_MAJOR_VERSION],[MOO_MODULE_MAJOR_VERSION])
AC_DEFINE_UNQUOTED(MOO_MODULE_MINOR_VERSION,[$MOO_MODULE_MINOR_VERSION],[MOO_MODULE_MINOR_VERSION])

AC_DEFINE_UNQUOTED(MOO_EMAIL,["$MOO_EMAIL"],MOO_EMAIL)
AC_DEFINE_UNQUOTED(PACKAGE_BUGREPORT,["$PACKAGE_BUGREPORT"],PACKAGE_BUGREPORT)
AC_DEFINE_UNQUOTED(MOO_COPYRIGHT,["$MOO_COPYRIGHT"],MOO_COPYRIGHT)
AC_DEFINE_UNQUOTED(MOO_WEBSITE,["$MOO_WEBSITE"],MOO_WEBSITE)
AC_DEFINE_UNQUOTED(MOO_WEB_CONTACT,["$MOO_WEB_CONTACT"],MOO_WEB_CONTACT)

MOO_AC_CHECK_OS

AC_SUBST(MOO_TOP_SRCDIR,`cd $srcdir && pwd`)

AM_CONDITIONAL(MOO_BUILD_CTAGS, [$MOO_OS_UNIX])
if $MOO_OS_UNIX; then
  AC_DEFINE(MOO_BUILD_CTAGS, 1, [build ctags plugin])
fi

if $MOO_OS_WIN32; then
  AC_SUBST(MEDIT_WIN32_APP_UID,"7F9F899F-EE8A-47F0-9981-8C525AF78E4D")
  AC_SUBST(MEDIT_INNO_TOP_SRCDIR,"Z:`cd $srcdir && pwd`")
  AC_SUBST(MEDIT_INNO_TOP_BUILDDIR,"Z:`pwd`")
  AC_SUBST(MEDIT_INNO_INSTDIR,"Z:`cd $prefix && pwd`")
  AC_SUBST(MEDIT_INNO_COMPILER,'wine "c:\\program files\\inno setup 5\\ISCC.exe"')
fi

AC_ARG_ENABLE(install-hooks,
  AC_HELP_STRING([--enable-install-hooks],[run gtk-update-icon-cache during 'make install']),[
  MOO_ENABLE_INSTALL_HOOKS="$enable_install_hooks"
  ],[
  MOO_ENABLE_INSTALL_HOOKS="yes"
])
AM_CONDITIONAL(MOO_ENABLE_INSTALL_HOOKS, test "x$MOO_ENABLE_INSTALL_HOOKS" = "xyes")

AC_CONFIG_FILES([
  Makefile
  doc/Makefile
  moo/Makefile
  po/Makefile
  po-gsv/Makefile
])
AC_OUTPUT
