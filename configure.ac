m4_define([_moo_major_version_],[0])
m4_define([_moo_minor_version_],[99])
m4_define([_moo_micro_version_],[0])
m4_define([_moo_version_suffix_],[unstable])
m4_define([_moo_module_major_version_],[2])
m4_define([_moo_module_minor_version_],[0])
m4_define([_moo_version_],[_moo_major_version_._moo_minor_version_._moo_micro_version_])

m4_if([_moo_version_suffix_],[],[m4_define([_moo_display_version_],[_moo_version_])],[m4_define([_moo_display_version_],[_moo_version_-_moo_version_suffix_])])

m4_define([_moo_email_],[emuntyan@sourceforge.net])
m4_define([_moo_copyright_],[2004-2010 Yevgen Muntyan <_moo_email_>])

AC_PREREQ([2.65])
AC_INIT([medit], [_moo_display_version_], [_moo_email_])
AC_GNU_SOURCE
AC_CONFIG_MACRO_DIR([m4])
AC_SUBST(ACLOCAL_FLAGS)

AM_INIT_AUTOMAKE([1.11 silent-rules foreign dist-bzip2 dist-zip no-dist-gzip])
LT_INIT([disable-shared])

AC_ARG_ENABLE(release,
  AC_HELP_STRING([--enable-release],[assume it's a release build]),[
  MOO_RELEASE_BUILD="$enable_release"
  ],[
  MOO_RELEASE_BUILD="no"
])
AM_CONDITIONAL(MOO_RELEASE_BUILD, test "x$MOO_RELEASE_BUILD" = "xMOO_RELEASE_BUILD")

MOO_MAJOR_VERSION=_moo_major_version_
MOO_MINOR_VERSION=_moo_minor_version_
MOO_MICRO_VERSION=_moo_micro_version_
MOO_MODULE_MAJOR_VERSION=_moo_module_major_version_
MOO_MODULE_MINOR_VERSION=_moo_module_minor_version_
MOO_VERSION=_moo_version_
MOO_DISPLAY_VERSION=_moo_display_version_
MOO_EMAIL="_moo_email_"
MOO_COPYRIGHT="_moo_copyright_"
MOO_PACKAGE_NAME=moo
AC_SUBST(MOO_EMAIL)
AC_SUBST(MOO_DISPLAY_VERSION)
AC_SUBST(MOO_COPYRIGHT)

GETTEXT_PACKAGE=moo
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE",[moo])
AM_GLIB_GNU_GETTEXT
IT_PROG_INTLTOOL
IT_PO_SUBDIR([po-gsv])

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

LT_LIB_M

AC_DEFINE(MOO_BUILD_APP,[1],[MOO_BUILD_APP])
AC_DEFINE(MOO_BUILD_EDIT,[1],[MOO_BUILD_EDIT])
AC_DEFINE(MOO_BUILD_LUA,[1],[MOO_BUILD_LUA])
AC_DEFINE(MOO_BUILD_UTILS,[1],[MOO_BUILD_UTILS])

AM_CONDITIONAL(MOO_BUILD_CTAGS, [$MOO_OS_UNIX])
if $MOO_OS_UNIX; then
  AC_DEFINE(MOO_BUILD_CTAGS, 1, [build ctags plugin])
fi

AC_DEFINE_UNQUOTED(MOO_VERSION,["$MOO_VERSION"],[MOO_VERSION])
AC_DEFINE_UNQUOTED(MOO_DISPLAY_VERSION,["$MOO_DISPLAY_VERSION"],[MOO_DISPLAY_VERSION])
AC_DEFINE_UNQUOTED(MOO_MAJOR_VERSION,[$MOO_MAJOR_VERSION],[MOO_MAJOR_VERSION])
AC_DEFINE_UNQUOTED(MOO_MICRO_VERSION,[$MOO_MICRO_VERSION],[MOO_MICRO_VERSION])
AC_DEFINE_UNQUOTED(MOO_MINOR_VERSION,[$MOO_MINOR_VERSION],[MOO_MINOR_VERSION])
AC_DEFINE_UNQUOTED(MOO_MODULE_MAJOR_VERSION,[$MOO_MODULE_MAJOR_VERSION],[MOO_MODULE_MAJOR_VERSION])
AC_DEFINE_UNQUOTED(MOO_MODULE_MINOR_VERSION,[$MOO_MODULE_MINOR_VERSION],[MOO_MODULE_MINOR_VERSION])

AC_DEFINE_UNQUOTED(MOO_EMAIL,["$MOO_EMAIL"],MOO_EMAIL)
AC_DEFINE_UNQUOTED(PACKAGE_BUGREPORT,["$PACKAGE_BUGREPORT"],PACKAGE_BUGREPORT)
AC_DEFINE_UNQUOTED(MOO_PACKAGE_NAME,["$MOO_PACKAGE_NAME"],MOO_PACKAGE_NAME)
AC_DEFINE_UNQUOTED(MOO_COPYRIGHT,["$MOO_COPYRIGHT"],MOO_COPYRIGHT)

MOO_AC_CHECK_OS

AC_CONFIG_FILES([
  Makefile
  doc/Makefile
  po/Makefile.in
  po-gsv/Makefile.in
  moo/Makefile
])
AC_OUTPUT
