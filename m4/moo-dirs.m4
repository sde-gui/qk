##############################################################################
# MOO_AC_SET_DIRS(base)
#
AC_DEFUN([MOO_AC_SET_DIRS],[
    MOO_PACKAGE_NAME=$1
    AC_SUBST(MOO_PACKAGE_NAME)
    AC_DEFINE([MOO_PACKAGE_NAME], "$1", [package name])

    MOO_DATA_DIR="${datadir}/$1"
    AC_SUBST(MOO_DATA_DIR)

    MOO_LIB_DIR="${libdir}/$1"
    AC_SUBST(MOO_LIB_DIR)

    MOO_PLUGINS_DIR="${MOO_LIB_DIR}/plugins"
    AC_SUBST(MOO_PLUGINS_DIR)

    # copied from po/Makefile.in.in
    MOO_LOCALE_DIR="${prefix}/${DATADIRNAME}/locale"
    AC_SUBST(MOO_LOCALE_DIR)

    MOO_TEXT_LANG_FILES_DIR="${MOO_DATA_DIR}/language-specs"
    AC_SUBST(MOO_TEXT_LANG_FILES_DIR)

    moo_includedir=${includedir}/$1
    AC_SUBST(moo_includedir)
])
