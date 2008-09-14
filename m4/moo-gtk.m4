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
  MOO_USE_GIO=true
  MOO_CHECK_VERSION(GIO, gio-2.0, [16, 18])
else
  AM_CONDITIONAL(GIO_2_16, false)
  AM_CONDITIONAL(GIO_2_18, false)

  # check for header files
  AC_CHECK_HEADERS([dirent.h float.h limits.h pwd.h grp.h sys/param.h sys/poll.h sys/resource.h])
  AC_CHECK_HEADERS([sys/time.h sys/times.h sys/wait.h unistd.h values.h])
  AC_CHECK_HEADERS([sys/select.h sys/types.h stdint.h sched.h malloc.h])
  AC_CHECK_HEADERS([sys/vfs.h sys/mount.h sys/vmount.h sys/statfs.h sys/statvfs.h])
  AC_CHECK_HEADERS([mntent.h sys/mnttab.h sys/vfstab.h sys/mntctl.h sys/sysctl.h fstab.h])

  # check for structure fields
  AC_CHECK_MEMBERS([struct stat.st_mtimensec, struct stat.st_mtim.tv_nsec, struct stat.st_atimensec, struct stat.st_atim.tv_nsec, struct stat.st_ctimensec, struct stat.st_ctim.tv_nsec])
  AC_CHECK_MEMBERS([struct stat.st_blksize, struct stat.st_blocks, struct statfs.f_fstypename, struct statfs.f_bavail],,, [#include <sys/types.h>
  #include <sys/stat.h>
  #include <unistd.h>
  #ifdef HAVE_SYS_STATFS_H
  #include <sys/statfs.h>
  #endif
  #ifdef HAVE_SYS_PARAM_H
  #include <sys/param.h>
  #endif
  #ifdef HAVE_SYS_MOUNT_H
  #include <sys/mount.h>
  #endif])
  # struct statvfs.f_basetype is available on Solaris but not for Linux.
  AC_CHECK_MEMBERS([struct statvfs.f_basetype],,, [#include <sys/statvfs.h>])

  # Check for some functions
  AC_CHECK_FUNCS(lstat strerror strsignal memmove vsnprintf stpcpy strcasecmp strncasecmp poll getcwd vasprintf setenv unsetenv getc_unlocked readlink symlink fdwalk)
  AC_CHECK_FUNCS(chown lchown fchmod fchown link statvfs statfs utimes getgrgid getpwuid)
  AC_CHECK_FUNCS(getmntent_r setmntent endmntent hasmntopt getmntinfo)

fi
AM_CONDITIONAL([MOO_USE_GIO], [$MOO_USE_GIO])
if $MOO_USE_GIO; then
  AC_DEFINE([MOO_USE_GIO], [1], [Use GIO library])
fi
])
