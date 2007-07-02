##############################################################################
# MOO_AC_PCRE
# This is essentially pcre's configure.in, contains checks and defines
# needed for pcre
#
AC_DEFUN_ONCE([MOO_AC_PCRE],[
AC_REQUIRE([MOO_PKG_CHECK_GTK_VERSIONS])

if test "x$GLIB_2_14" = xyes; then
  MOO_BUILD_PCRE="no"
else
  AC_ARG_WITH([system-pcre],
  AC_HELP_STRING([--with-system-pcre], [whether to use system copy of pcre library (default = YES)]),[
    if test x$with_system_pcre = "xyes"; then
      MOO_BUILD_PCRE="no"
    else
      MOO_BUILD_PCRE="yes"
    fi
  ],[
    MOO_BUILD_PCRE="auto"
  ])
fi;

if test x$MOO_BUILD_PCRE != xyes; then
    have_pcre="no"

    PKG_CHECK_MODULES(PCRE, [libpcre >= 7.0], [
        have_pcre="yes"
    ], [
        have_pcre="no"
    ])

    if test $have_pcre = yes; then
        AC_MSG_CHECKING(pcre UTF8 support)

        _moo_ac_pcre_libs=`$PKG_CONFIG --libs-only-l libpcre`
        _moo_ac_pcre_ldflags=`$PKG_CONFIG --libs-only-L libpcre`
        moo_ac_save_CPPFLAGS="$CPPFLAGS"
        CPPFLAGS="$CPPFLAGS $PCRE_CFLAGS"
        moo_ac_save_CFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS $PCRE_CFLAGS"
        moo_ac_save_LDFLAGS="$LDFLAGS"
        LDFLAGS="$LDFLAGS $_moo_ac_pcre_ldflags"
        moo_ac_save_LIBS="$LIBS"
        LIBS="$LIBS $_moo_ac_pcre_libs"

        AC_RUN_IFELSE([
            AC_LANG_SOURCE([[
            #include <pcre.h>
            #include <stdlib.h>
            int main (int argc, char *argv[])
            {
                int result = 0;

                pcre_config (PCRE_CONFIG_UTF8, &result);

                if (result)
                    pcre_config (PCRE_CONFIG_UNICODE_PROPERTIES, &result);

                if (result)
                    exit (0);
                else
                    exit (1);
            }]
        ])],[
            AC_MSG_RESULT(yes)
            have_pcre=yes
        ],[
            AC_MSG_RESULT(no)
            have_pcre=no
        ],[
            AC_MSG_RESULT(can't check when crosscompiling, assuming it's fine)
            have_pcre=yes
        ])

        CFLAGS="$moo_ac_save_CFLAGS"
        CPPFLAGS="$moo_ac_save_CPPFLAGS"
        LDFLAGS="$moo_ac_save_LDFLAGS"
        LIBS="$moo_ac_save_LIBS"
    fi
fi

if test x$MOO_BUILD_PCRE != xyes; then
    if test x$have_pcre = xyes; then
        MOO_BUILD_PCRE="no"
        AC_MSG_NOTICE([using installed libpcre])
        MOO_PCRE_CFLAGS="$PCRE_CFLAGS"
        MOO_PCRE_LIBS="$PCRE_LIBS"
    else
        MOO_BUILD_PCRE="yes"
        AC_MSG_NOTICE([building pcre library])
    fi
else
    MOO_BUILD_PCRE="yes"
    AC_MSG_NOTICE([building pcre library])
fi

AM_CONDITIONAL(MOO_BUILD_PCRE, test x$MOO_BUILD_PCRE = xyes)
if test x$MOO_BUILD_PCRE = xyes; then
    AC_DEFINE(MOO_BUILD_PCRE, , [MOO_BUILD_PCRE - build pcre library])
fi

]) # end of MOO_AC_PCRE
