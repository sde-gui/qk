##############################################################################
# MOO_AC_CHECK_OS([])
#
AC_DEFUN([MOO_AC_CHECK_OS],[
AC_REQUIRE([AC_CANONICAL_HOST])
m4_foreach([name], [CYGWIN, MINGW, DARWIN, FREEBSD, UNIX, BSD, LINUX], [MOO_OS_[]name=""; ])

case $host in
*-*-mingw32*)
  MOO_OS_MINGW="yes"
  MOO_OS_NAME="Win32"
  ;;
*-*-cygwin*)
  MOO_OS_CYGWIN="yes"
  MOO_OS_NAME="CygWin"
  ;;
*-*-darwin*)
  MOO_OS_DARWIN="yes"
  MOO_OS_NAME="Darwin"
  ;;
*-*-freebsd*)
  MOO_OS_FREEBSD="yes"
  MOO_OS_NAME="FreeBSD"
  ;;
*-*-linux*)
  MOO_OS_LINUX="yes"
  MOO_OS_NAME="Linux"
  ;;
*)
  MOO_OS_UNIX="yes"
  MOO_OS_NAME="Unix"
  ;;
esac

if test x$MOO_OS_DARWIN = xyes; then MOO_OS_BSD=yes; fi
if test x$MOO_OS_FREEBSD = xyes; then MOO_OS_BSD=yes; fi
if test x$MOO_OS_BSD = xyes; then MOO_OS_UNIX=yes; fi
if test x$MOO_OS_LINUX = xyes; then MOO_OS_UNIX=yes; fi

m4_foreach([name], [CYGWIN, MINGW, DARWIN, UNIX, FREEBSD, BSD, LINUX], [dnl
if test x$MOO_OS_[]name = xyes; then
  AC_DEFINE(MOO_OS_[]name, 1, [name])
fi
AM_CONDITIONAL(MOO_OS_[]name, test x$MOO_OS_[]name = xyes)
])
])
