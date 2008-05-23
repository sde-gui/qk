AC_DEFUN_ONCE([MOO_AM_MIME_MK],[
  MOO_ENABLE_GENERATED_FILES="yes"

  AC_ARG_ENABLE(generated-files,
    AC_HELP_STRING([--enable-generated-files],[run update-mime-database and gtk-update-icon-cache on install (default = YES)]),[
    if test "x$enable_generated_files" = "xno"; then
      MOO_ENABLE_GENERATED_FILES="no"
    fi ])
  AM_CONDITIONAL(MOO_ENABLE_GENERATED_FILES, test x$MOO_ENABLE_GENERATED_FILES = "xyes")

  mimedir="${datadir}/mime"
  AC_SUBST(mimedir)
])

AC_DEFUN_ONCE([MOO_AM_RC_MK],[
  AC_REQUIRE([MOO_AC_CHECK_OS])
  if test x$MOO_OS_MINGW = "xyes"; then
    AC_ARG_VAR([WINDRES], [windres])
    AC_CHECK_TOOL(WINDRES, windres, :)
  fi
])

AC_DEFUN_ONCE([MOO_AM_MK],[
  AC_REQUIRE([MOO_AC_I18N])
  AC_REQUIRE([MOO_AC_SRCDIR])
  MOO_AM_MIME_MK
  MOO_AM_RC_MK
])
