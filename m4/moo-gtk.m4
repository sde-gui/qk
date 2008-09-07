##############################################################################
# _MOO_SPLIT_VERSION_PKG(PKG_NAME,pkg-name)
#
AC_DEFUN([_MOO_SPLIT_VERSION_PKG],[
AC_MSG_CHECKING($1 version)
_moo_ac_version=`$PKG_CONFIG --modversion $2`
_MOO_SPLIT_VERSION([$1],[$_moo_ac_version])
AC_MSG_RESULT($[]$1[]_MAJOR_VERSION.$[]$1[]_MINOR_VERSION.$[]$1[]_MICRO_VERSION)
])


##############################################################################
# MOO_CHECK_VERSION(PKG_NAME,pkg-name,versions)
#
AC_DEFUN([MOO_CHECK_VERSION],[
PKG_CHECK_MODULES($1,$2)
_MOO_SPLIT_VERSION_PKG($1,$2)
m4_foreach([num],[$3],
[AM_CONDITIONAL($1[]_2_[]num, test $[]$1[]_MINOR_VERSION -ge num)
 if test $[]$1[]_MINOR_VERSION -ge num; then
   $1[]_2_[]num=yes
 fi
])
])


##############################################################################
# _MOO_CHECK_BROKEN_GTK_THEME
#
AC_DEFUN([_MOO_CHECK_BROKEN_GTK_THEME],[
AC_ARG_WITH([broken-gtk-theme], AC_HELP_STRING([--with-broken-gtk-theme], [Work around bug in gtk theme (Suse 9 has one)]), [
  if test x$with_broken_gtk_theme = "xyes"; then
    MOO_BROKEN_GTK_THEME="yes"
  fi
])

if test x$MOO_BROKEN_GTK_THEME = xyes; then
  AC_MSG_NOTICE([Broken gtk theme])
  AC_DEFINE(MOO_BROKEN_GTK_THEME, 1, [broken gtk theme])
fi
])


##############################################################################
# MOO_PKG_CHECK_GTK_VERSIONS
#
AC_DEFUN_ONCE([MOO_PKG_CHECK_GTK_VERSIONS],[
AC_REQUIRE([MOO_AC_CHECK_OS])
MOO_CHECK_VERSION(GTK, gtk+-2.0, [6, 10])
MOO_CHECK_VERSION(GLIB, glib-2.0, [8, 12, 14, 16]) # DO NOT EVER EVER REMOVE VERSIONS HERE!
PKG_CHECK_MODULES(GTHREAD, gthread-2.0)
_MOO_CHECK_BROKEN_GTK_THEME

gdk_target=`$PKG_CONFIG --variable=target gdk-2.0`

GDK_X11=false
GDK_WIN32=false
GDK_QUARTZ=false

case $gdk_target in
x11)
  GDK_X11=true
  ;;
quartz)
  GDK_QUARTZ=true
  ;;
win32)
  GDK_WIN32=true
  ;;
esac

AM_CONDITIONAL(GDK_X11, $GDK_X11)
AM_CONDITIONAL(GDK_WIN32, $GDK_WIN32)
AM_CONDITIONAL(GDK_QUARTZ, $GDK_QUARTZ)

MOO_USE_GIO=false
if test "x$GLIB_2_16" = xyes; then
  PKG_CHECK_MODULES(GIO,[gio-2.0],[
    MOO_USE_GIO=true
    MOO_CHECK_VERSION(GIO, gio-2.0, [16, 18])
  ],[:])
fi
AM_CONDITIONAL([MOO_USE_GIO], [$MOO_USE_GIO])
if $MOO_USE_GIO; then
  AC_DEFINE([MOO_USE_GIO], [1], [Use GIO library])
fi
])
