##############################################################################
# MOO_AC_FUNCS()
# various checks needed in moo
#
AC_DEFUN_ONCE([MOO_AC_FUNCS],[
    # for xdgmime
    AC_CHECK_FUNCS(getc_unlocked)

    # for GMappedFile and xdgmime
    AC_CHECK_FUNCS(mmap)

    AC_CHECK_HEADERS(unistd.h)

    # for mooappabout.h
    AC_CHECK_HEADERS(sys/utsname.h)

    # for mooapp.c
    AC_CHECK_HEADERS(signal.h)

    ################################################################################
    #  Terminal stuff
    AC_CHECK_HEADERS([poll.h sys/wait.h])
    AC_CHECK_HEADERS([sys/termios.h sys/un.h stropts.h termios.h])
    AC_CHECK_HEADERS([libutil.h util.h])
    AC_CHECK_FUNCS([signal posix_openpt])
    AC_CHECK_FUNCS([getpt grantpt unlockpt ptsname ptsname_r])
    AC_TYPE_PID_T
    AC_HEADER_TIOCGWINSZ
    ################################################################################
])
