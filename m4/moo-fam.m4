##############################################################################
# MOO_AC_CHECK_FAM(action-if-found,action-if-not-found)
#
AC_DEFUN([MOO_AC_CHECK_FAM],[
    save_CFLAGS="$CFLAGS"
    save_LDFLAGS="$LDFLAGS"

    CFLAGS="$CFLAGS $FAM_CFLAGS"
    if test x$FAM_LIBS = x; then
        FAM_LIBS=-lfam
    fi
    LDFLAGS="$LDFLAGS $FAM_LIBS"

    AC_CHECK_HEADERS(fam.h,[
        AC_CHECK_FUNCS([FAMMonitorDirectory FAMOpen],[fam_found=yes],[fam_found=no])
    ],[fam_found=no])

    if test x$fam_found != xno; then
        AC_SUBST(FAM_CFLAGS)
        AC_SUBST(FAM_LIBS)

        AC_MSG_CHECKING(for FAM_CFLAGS)
        if test -z $FAM_CFLAGS; then
            AC_MSG_RESULT(None)
        else
            AC_MSG_RESULT($FAM_CFLAGS)
        fi

        AC_MSG_CHECKING(for FAM_LIBS)
        if test -z $FAM_LIBS; then
            AC_MSG_RESULT(None)
        else
            AC_MSG_RESULT($FAM_LIBS)
        fi

        MOO_FAM_LIBS=$FAM_LIBS
        ifelse([$1], , :, [$1])
    else
        unset FAM_CFLAGS
        unset FAM_LIBS
        MOO_FAM_LIBS=
        ifelse([$2], , [AC_MSG_ERROR(libfam not found)], [$2])
    fi

    AC_SUBST(MOO_FAM_LIBS)
    CFLAGS="$save_CFLAGS"
    LDFLAGS="$save_LDFLAGS"
])


AC_DEFUN([MOO_AC_FAM],[
    AC_REQUIRE([MOO_AC_CHECK_OS])

#    AC_ARG_WITH([fam], AC_HELP_STRING([--with-fam], [whether to use fam or gamin for monitoring files in the editor (default = NO)]), [
#            if test x$with_fam = "xyes"; then
#                MOO_USE_FAM="yes"
#            else
#                MOO_USE_FAM="no"
#            fi
#        ],[
#            MOO_USE_FAM="no"
#    ])

#    if test x$MOO_OS_UNIX = xyes -a x$MOO_USE_FAM = xyes; then
#        MOO_AC_CHECK_FAM([moo_has_fam=yes],[moo_has_fam=no])
#        if test x$moo_has_fam = xyes; then
#            MOO_USE_FAM="yes"
#            AC_DEFINE(MOO_USE_FAM, 1, [use libfam for monitoring files])
#        else
#            AC_MSG_ERROR([FAM or gamin not found.])
#        fi
#    fi

    MOO_USE_FAM=no
    AM_CONDITIONAL(MOO_USE_FAM, test x$MOO_USE_FAM = "xyes")
])
