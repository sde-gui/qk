##############################################################################
# MOO_AC_SET_DIRS
#
AC_DEFUN_ONCE([MOO_AC_SET_DIRS],[
  if test "x$MOO_PACKAGE_NAME" = x; then
    MOO_PACKAGE_NAME=moo
  fi

  AC_SUBST(MOO_PACKAGE_NAME)
  AC_DEFINE_UNQUOTED([MOO_PACKAGE_NAME], "$MOO_PACKAGE_NAME", [package name])

  MOO_DATA_DIR="${datadir}/$MOO_PACKAGE_NAME"
  AC_SUBST(MOO_DATA_DIR)

  MOO_LIB_DIR="${libdir}/$MOO_PACKAGE_NAME"
  AC_SUBST(MOO_LIB_DIR)

  mimedir="${datadir}/mime"
  AC_SUBST(mimedir)

  MOO_PLUGINS_DIR="${MOO_LIB_DIR}/plugins"
  AC_SUBST(MOO_PLUGINS_DIR)

  MOO_TEXT_LANG_FILES_DIR="${MOO_DATA_DIR}/language-specs"
  AC_SUBST(MOO_TEXT_LANG_FILES_DIR)

  moo_includedir=${includedir}/$MOO_PACKAGE_NAME
  AC_SUBST(moo_includedir)

  MOO_LOCALE_DIR="${datadir}/locale"
  AC_SUBST(MOO_LOCALE_DIR)

  MOO_HELP_DIR="${htmldir}/help"
  AC_SUBST(MOO_HELP_DIR)
])
