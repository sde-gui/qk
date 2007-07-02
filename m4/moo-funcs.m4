##############################################################################
# MOO_AC_FUNCS()
# various checks needed in moo
#
AC_DEFUN_ONCE([MOO_AC_FUNCS],[
    # backtrace() and friends
    AC_CHECK_HEADERS(execinfo.h)
    AC_CHECK_FUNCS(backtrace)

    # for xdgmime
    AC_CHECK_HEADERS(fnmatch.h netinet/in.h)

    #for GMappedFile
    AC_CHECK_HEADERS(unistd.h)
    AC_CHECK_FUNCS(mmap)

    #for moofileutils.c
    AC_CHECK_FUNCS(unlink)

    #for mooappabout.h
    AC_CHECK_HEADERS(sys/utsname.h)

    #for mooapp.c
    AC_CHECK_HEADERS(signal.h)

    ################################################################################
    #  Terminal stuff
    AC_CHECK_HEADERS([poll.h errno.h io.h fcntl.h sys/types.h sys/wait.h signal.h])
    AC_CHECK_HEADERS([sys/select.h sys/termios.h sys/un.h stropts.h termios.h wchar.h])
    AC_CHECK_HEADERS([libutil.h util.h])
    AC_CHECK_FUNCS([poll pipe _pipe kill signal posix_openpt])
    AC_CHECK_FUNCS([cfmakeraw getpgid getpt grantpt unlockpt ptsname ptsname_r recvmsg])
    AC_TYPE_PID_T
    AC_HEADER_TIOCGWINSZ
    ################################################################################
])
