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
        datadir=${prefix}/share
    fi
    if test "x{libdir}" = "xNONE"; then
        libdir=${prefix}/lib
    fi

    AC_DEFINE_UNQUOTED(MOO_DATA_DIR, "${datadir}/$1", [data dir])
    AC_DEFINE_UNQUOTED(MOO_LIB_DIR, "${libdir}/$1", [lib dir])

    MOO_TEXT_LANG_FILES_DIR="${datadir}/$1/syntax"
    AC_SUBST(MOO_TEXT_LANG_FILES_DIR)
    AC_DEFINE_UNQUOTED(MOO_TEXT_LANG_FILES_DIR, "${datadir}/$1/syntax", [lang files dir])
])
