##############################################################################
# _MOO_AC_CHECK_FAM(action-if-found,action-if-not-found)
#
AC_DEFUN_ONCE([_MOO_AC_CHECK_FAM],[
    moo_ac_save_CFLAGS="$CFLAGS"
    moo_ac_save_LIBS="$LIBS"

    if test x$FAM_LIBS = x; then
        FAM_LIBS=-lfam
    fi

    CFLAGS="$CFLAGS $FAM_CFLAGS"
    LIBS="$LIBS $FAM_LIBS"

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

        AC_CHECK_DECL([FAMNoExists],[
          AC_DEFINE(HAVE_FAMNOEXISTS, 1, [fam.h has FAMNoExists defined])
          AC_DEFINE(MOO_USE_GAMIN, 1, [whether libfam is provided by gamin])
        ],[],[#include <fam.h>])

        MOO_FAM_CFLAGS="$FAM_CFLAGS"
        MOO_FAM_LIBS="$FAM_LIBS"
        ifelse([$1], , :, [$1])
    else
        unset FAM_CFLAGS
        unset FAM_LIBS
        MOO_FAM_LIBS=
        MOO_FAM_CFLAGS=
        ifelse([$2], , [AC_MSG_ERROR(libfam not found)], [$2])
    fi

    AC_SUBST(MOO_FAM_CFLAGS)
    AC_SUBST(MOO_FAM_LIBS)
    CFLAGS="$moo_ac_save_CFLAGS"
    LIBS="$moo_ac_save_LIBS"
])


AC_DEFUN_ONCE([MOO_AC_FAM],[
   AC_REQUIRE([MOO_AC_CHECK_OS])

   AC_ARG_WITH([fam], AC_HELP_STRING([--with-fam], [whether to use fam or gamin for monitoring files in the editor (default = NO)]), [
           if test x$with_fam = "xyes"; then
               MOO_USE_FAM="yes"
           else
               MOO_USE_FAM="no"
           fi
       ],[
           MOO_USE_FAM="no"
   ])

   if test x$MOO_OS_UNIX = xyes -a x$MOO_USE_FAM = xyes; then
       _MOO_AC_CHECK_FAM([moo_has_fam=yes],[moo_has_fam=no])
       if test x$moo_has_fam = xyes; then
           MOO_USE_FAM="yes"
           AC_DEFINE(MOO_USE_FAM, 1, [use libfam for monitoring files])
       else
           AC_MSG_ERROR([FAM or gamin not found.])
       fi
   fi

    AM_CONDITIONAL(MOO_USE_FAM, test x$MOO_USE_FAM = "xyes")
])
