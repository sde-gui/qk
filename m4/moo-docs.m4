AC_DEFUN([_MOO_AC_CHECK_XSLT_DOCBOOK],[
  AC_ARG_VAR([XSLTPROC],[path to xsltproc utility])
  AC_CHECK_PROG([XSLTPROC],[xsltproc],[xsltproc])

  if test -z "$XSLTPROC"; then
    $2
  fi

  if test -n "$XSLTPROC"; then
    AC_MSG_CHECKING([whether xsltproc works])
    cat > conftest.docbook << EOFEOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE book PUBLIC "-//OASIS//DTD DocBook XML V4.4//EN"
                    "http://www.oasis-open.org/docbook/xml/4.4/docbookx.dtd"
>
<article>
<articleinfo>
<title>Title</title>
</articleinfo>
<sect1>
<title>A section</title>
<para>Blah blah blah.</para>
</sect1>
</article>
EOFEOF
    if ($XSLTPROC http://docbook.sourceforge.net/release/xsl/current/html/docbook.xsl conftest.docbook 2>/dev/null >/dev/null); then
      AC_MSG_RESULT([yes])
      rm -f conftest.docbook
      $1
    else
      AC_MSG_RESULT([no])
      rm -f conftest.docbook
      $2
    fi
  fi
])

AC_DEFUN([MOO_DOCS],[
  AC_REQUIRE([MOO_AC_SET_DIRS])

  AC_ARG_ENABLE([help],
    AC_HELP_STRING(--disable-help, [Disable building html help files (default = auto).]),
    [:],[enable_help=auto])

  if test "x$enable_help" = xauto -o "x$enable_help" = xyes; then
    _MOO_AC_CHECK_XSLT_DOCBOOK([_moo_xslt_working=yes],[_moo_xslt_working=no])
    if test $_moo_xslt_working = yes; then
      enable_help=yes
    elif test "x$enable_help" = xauto; then
      enable_help=no
    else
      AC_MSG_ERROR([xsltproc will not work])
    fi
  fi

  AM_CONDITIONAL(MOO_ENABLE_HELP, test "x$enable_help" = xyes)
  if test "x$enable_help" = xyes; then
    AC_DEFINE(MOO_ENABLE_HELP, [1], [enable help functionality])
  fi
])
