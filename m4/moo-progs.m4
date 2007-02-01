##############################################################################
# MOO_AC_PROGS
# checks for prgoams needed
#
AC_DEFUN([MOO_AC_PROGS],[
    AC_ARG_VAR([WINDRES], [windres])
    AC_CHECK_TOOL(WINDRES, windres, :)
    AC_ARG_VAR([OPAG], [opag])
    AC_CHECK_PROG(OPAG, opag, opag)
])
