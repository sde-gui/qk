##############################################################################
# _MOO_SPLIT_VERSION(NAME,version)
#
AC_DEFUN([_MOO_SPLIT_VERSION],[AC_REQUIRE([LT_AC_PROG_SED])
$1[]_VERSION="$2"
$1[]_MAJOR_VERSION=`echo "$2" | $SED 's/\([[^.]][[^.]]*\).*/\1/'`
$1[]_MINOR_VERSION=`echo "$2" | $SED 's/[[^.]][[^.]]*.\([[^.]][[^.]]*\).*/\1/'`
$1[]_MICRO_VERSION=`echo "$2" | $SED 's/[[^.]][[^.]]*.[[^.]][[^.]]*.\(.*\)/\1/'`
])

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
# MOO_CHECK_VERSION(PKG_NAME,pkg-name)
#
AC_DEFUN([MOO_CHECK_VERSION],[
PKG_CHECK_MODULES($1,$2)
# _MOO_SPLIT_VERSION_PKG($1,$2)
# m4_foreach([num],[2,4,6,8,10,12,14],
# [AM_CONDITIONAL($1[]_2_[]num, test $[]$1[]_MINOR_VERSION -ge num)
# if test $[]$1[]_MINOR_VERSION -ge num; then
#   $1[]_2_[]num=yes
# fi
# ])
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
MOO_CHECK_VERSION(GTK, gtk+-2.0)
# MOO_CHECK_VERSION(GLIB, glib-2.0)
MOO_CHECK_VERSION(GTHREAD, gthread-2.0)
# MOO_CHECK_VERSION(GDK, gdk-2.0)

MOO_CHECK_VERSION(XML, libxml-2.0)
AC_DEFINE(MOO_USE_XML, 1, [use libxml2])

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

GLIB_GENMARSHAL=`$PKG_CONFIG --variable=glib_genmarshal glib-2.0`
GLIB_MKENUMS=`$PKG_CONFIG --variable=glib_mkenums glib-2.0`
AC_SUBST(GLIB_GENMARSHAL)
AC_SUBST(GLIB_MKENUMS)

AC_ARG_VAR([GDK_PIXBUF_CSOURCE], [gdk-pixbuf-csource])
AC_CHECK_TOOL(GDK_PIXBUF_CSOURCE, gdk-pixbuf-csource, [AC_MSG_ERROR([gdk-pixbuf-csource not found])])

])
