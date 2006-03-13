##############################################################################
# MOO_AC_SET_DIRS(base)
#
AC_DEFUN([MOO_AC_SET_DIRS],[
    datadir=share
    libdir=lib

    if test "x${prefix}" = "xNONE"; then
        prefix=${ac_default_prefix}
    fi

    if test "x{datadir}" = "xNONE"; then
        datadir=${ac_default_datadir}
    fi
    if test "x{libdir}" = "xNONE"; then
        libdir=${ac_default_libdir}
    fi

    MOO_DATA_DIR="${prefix}/${datadir}/$1"
    AC_SUBST(MOO_DATA_DIR)
    AC_DEFINE_UNQUOTED(MOO_DATA_DIR, "${MOO_DATA_DIR}", [data dir])

    MOO_LIB_DIR="${prefix}/${datadir}/$1"
    AC_SUBST(MOO_LIB_DIR)
    AC_DEFINE_UNQUOTED(MOO_LIB_DIR, "${MOO_LIB_DIR}", [lib dir])

    MOO_TEXT_LANG_FILES_DIR="${MOO_DATA_DIR}/syntax"
    AC_SUBST(MOO_TEXT_LANG_FILES_DIR)
    AC_DEFINE_UNQUOTED(MOO_TEXT_LANG_FILES_DIR, "${MOO_TEXT_LANG_FILES_DIR}", [lang files dir])
])
