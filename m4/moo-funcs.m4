##############################################################################
# MOO_AC_FUNCS()
# various checks needed in moo
#
AC_DEFUN([MOO_AC_FUNCS],[
    # for pcre
    AC_CHECK_FUNCS(memmove bcopy strerror)

    #for GMappedFile
    AC_CHECK_HEADERS(unistd.h)
    AC_CHECK_FUNCS(mmap)

    #for moofileutils.c
    AC_CHECK_FUNCS(unlink)

    ################################################################################
    #  Terminal stuff
    AC_CHECK_HEADERS([poll.h errno.h io.h fcntl.h sys/types.h sys/wait.h signal.h])
    AC_CHECK_HEADERS([sys/select.h sys/termios.h sys/un.h stropts.h termios.h wchar.h])
    AC_CHECK_FUNCS([poll pipe _pipe kill signal posix_openpt])
    AC_CHECK_FUNCS([cfmakeraw getpgid getpt grantpt unlockpt ptsname ptsname_r recvmsg])
    AC_TYPE_SIGNAL
    AC_TYPE_SIZE_T
    AC_TYPE_PID_T
    AC_HEADER_TIOCGWINSZ
    ################################################################################
])
