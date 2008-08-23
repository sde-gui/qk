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
  AC_CHECK_FUNCS([signal])

  # for mdfileops.c
  AC_CHECK_FUNCS([link fchown fchmod])

  # for mooutils-fs.c
  AC_CHECK_HEADERS(sys/wait.h)
])
