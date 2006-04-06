##############################################################################
# MOO_AC_RELOC()
#
AC_DEFUN([MOO_AC_RELOC],[
    AC_REQUIRE([MOO_AC_CHECK_OS])

    if test "x$MOO_OS_MINGW" = "xyes"; then
        MOO_ENABLE_RELOCATION="yes"
    else
        MOO_ENABLE_RELOCATION="no"
    fi

    AC_ARG_ENABLE(relocation,
    AC_HELP_STRING([--enable-relocation],[do not hardcode data directories (default = NO)]),[
        if test "x$enableval" = "xyes"; then
            MOO_ENABLE_RELOCATION="yes"
        else
            MOO_ENABLE_RELOCATION="no"
        fi
    ])

    AM_CONDITIONAL(MOO_ENABLE_RELOCATION, test $MOO_ENABLE_RELOCATION = "yes")

    if test "x$MOO_ENABLE_RELOCATION" = "xyes"; then
        AC_DEFINE(MOO_ENABLE_RELOCATION, 1, [enable relocation])
    fi
])
