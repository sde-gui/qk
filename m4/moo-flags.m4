##############################################################################
# MOO_AC_FLAGS
#
AC_DEFUN([MOO_AC_FLAGS],[
    AC_REQUIRE([MOO_AC_FAM])
    AC_REQUIRE([MOO_PKG_CHECK_GTK_VERSIONS])
    AC_REQUIRE([MOO_AC_SET_DIRS])

    moo_top_src_dir=`cd $srcdir && pwd`
    moo_top_build_dir=`cd ./$ac_top_builddir && pwd`
    MOO_CFLAGS="-I$moo_top_src_dir/moo $GTK_CFLAGS -DXDG_PREFIX=_moo_edit_xdg -DG_LOG_DOMAIN=\\\"Moo\\\" -D__MOO__ -DMOO_MARSHALS_H=\\\"mooutils/moomarshals.h\\\""
    MOO_LIBS="$GTK_LIBS"

    if test x$MOO_USE_FAM = xyes; then
        MOO_LIBS="$MOO_LIBS $FAM_LIBS"
    fi

    MOO_CFLAGS="$MOO_CFLAGS -DMOO_DATA_DIR=\\\"${MOO_DATA_DIR}\\\" -DMOO_LIB_DIR=\\\"${MOO_LIB_DIR}\\\""

    ################################################################################
    #  MooEdit stuff
    #
    if test "x$build_mooedit" != "xno"; then
        MOO_CFLAGS="$MOO_CFLAGS $XML_CFLAGS -DMOO_TEXT_LANG_FILES_DIR=\\\"${MOO_TEXT_LANG_FILES_DIR}\\\""
        MOO_LIBS="$MOO_LIBS $XML_LIBS"
    fi

    AC_SUBST(MOO_CFLAGS)
    AC_SUBST(MOO_LIBS)
])
