AC_DEFUN_ONCE([MOO_AM_MIME_MK],[
  MOO_ENABLE_GENERATED_FILES="yes"

  AC_ARG_ENABLE(generated-files,
    AC_HELP_STRING([--enable-generated-files],[run gtk-update-icon-cache on install (default = YES)]),[
    if test "x$enable_generated_files" = "xno"; then
      MOO_ENABLE_GENERATED_FILES="no"
    fi ])
  AM_CONDITIONAL(MOO_ENABLE_GENERATED_FILES, test x$MOO_ENABLE_GENERATED_FILES = "xyes")
])

AC_DEFUN_ONCE([MOO_AM_RC_MK],[
  AC_REQUIRE([MOO_AC_CHECK_OS])
  if test x$MOO_OS_MINGW = "xyes"; then
    AC_ARG_VAR([WINDRES], [windres])
    AC_CHECK_TOOL(WINDRES, windres, :)
  fi
])

dnl _MOO_AC_CHECK_TOOL(variable,program)
AC_DEFUN([_MOO_AC_CHECK_TOOL],[
  AC_ARG_VAR([$1], [$2 program])
  AC_PATH_PROG([$1], [$2], [])
  AM_CONDITIONAL([HAVE_$1],[ test "x$2" != "x" ])
])

AC_DEFUN_ONCE([MOO_AC_CHECK_TOOLS],[
  _MOO_AC_CHECK_TOOL([GDK_PIXBUF_CSOURCE], [gdk-pixbuf-csource])
  _MOO_AC_CHECK_TOOL([GLIB_GENMARSHAL], [glib-genmarshal])
  _MOO_AC_CHECK_TOOL([GLIB_MKENUMS], [glib-mkenums])
  _MOO_AC_CHECK_TOOL([TXT2TAGS], [txt2tags])
])

AC_DEFUN_ONCE([MOO_AM_MK],[
  AC_REQUIRE([MOO_AC_I18N])
  MOO_AM_MIME_MK
  MOO_AM_RC_MK
  MOO_AC_CHECK_TOOLS
])
