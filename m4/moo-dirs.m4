##############################################################################
# MOO_AC_SET_DIRS(base)
#
AC_DEFUN([MOO_AC_SET_DIRS],[
    MOO_DATA_DIR="${datadir}/$1"
    AC_SUBST(MOO_DATA_DIR)

    MOO_LIB_DIR="${libdir}/$1"
    AC_SUBST(MOO_LIB_DIR)

    MOO_TEXT_LANG_FILES_DIR="${MOO_DATA_DIR}/syntax"
    AC_SUBST(MOO_TEXT_LANG_FILES_DIR)
])
