##############################################################################
# MOO_AC_PCRE
# This is essentially pcre's configure.in, contains checks and defines
# needed for pcre
#
AC_DEFUN([MOO_AC_PCRE],[

dnl Provide the current PCRE version information. Do not use numbers
dnl with leading zeros for the minor version, as they end up in a C
dnl macro, and may be treated as octal constants. Stick to single
dnl digits for minor numbers less than 10. There are unlikely to be
dnl that many releases anyway.

PCRE_MAJOR=6
PCRE_MINOR=4
PCRE_DATE=05-Sep-2005
PCRE_VERSION=${PCRE_MAJOR}.${PCRE_MINOR}

dnl Default values for miscellaneous macros

AC_DEFINE(POSIX_MALLOC_THRESHOLD, 10, [POSIX_MALLOC_THRESHOLD])

AC_HEADER_STDC
AC_CHECK_HEADERS(limits.h)

dnl Checks for typedefs, structures, and compiler characteristics.

AC_C_CONST
AC_TYPE_SIZE_T

AC_CHECK_TYPES([long long], [pcre_have_long_long="1"], [pcre_have_long_long="0"])
AC_CHECK_TYPES([unsigned long long], [pcre_have_ulong_long="1"], [pcre_have_ulong_long="0"])
AC_SUBST(pcre_have_long_long)
AC_SUBST(pcre_have_ulong_long)

dnl Checks for library functions.

AC_CHECK_FUNCS(bcopy memmove strerror strtoq strtoll)

dnl Handle --enable-utf8
AC_DEFINE(SUPPORT_UTF8, , [SUPPORT_UTF8])

# XXX
# dnl Handle --enable-unicode-properties
AC_DEFINE(SUPPORT_UCP, , [SUPPORT_UCP])


AC_DEFINE(PCRE_EXPORT, , [PCRE_EXPORT - empty since we do not need pcre api be exported])

# dnl Handle --enable-newline-is-cr
#
# AC_ARG_ENABLE(newline-is-cr,
# [  --enable-newline-is-cr  use CR as the newline character],
# if test "$enableval" = "yes"; then
#   NEWLINE=-DNEWLINE=13
# fi
# )
#
# dnl Handle --enable-newline-is-lf
#
# AC_ARG_ENABLE(newline-is-lf,
# [  --enable-newline-is-lf  use LF as the newline character],
# if test "$enableval" = "yes"; then
#   NEWLINE=-DNEWLINE=10
# fi
# )
AC_DEFINE(NEWLINE, '\n', [The value of NEWLINE determines the newline character])

# XXX
# dnl Handle --enable-ebcdic
#
# AC_ARG_ENABLE(ebcdic,
# [  --enable-ebcdic         assume EBCDIC coding rather than ASCII],
# if test "$enableval" == "yes"; then
#   EBCDIC=-DEBCDIC=1
# fi
# )
AC_DEFINE(EBCDIC, 0, [If you are compiling for a system that uses EBCDIC instead of ASCII dnl
character codes, define this macro as 1.])


# dnl Handle --disable-stack-for-recursion
#
# AC_ARG_ENABLE(stack-for-recursion,
# [  --disable-stack-for-recursion  disable use of stack recursion when matching],
# if test "$enableval" = "no"; then
#   NO_RECURSE=-DNO_RECURSE
# fi
# )

# dnl Handle --with-link-size=n
# AC_ARG_WITH(link-size,
# [  --with-link-size=2    internal link size (2, 3, or 4 allowed)],
#   LINK_SIZE=-DLINK_SIZE=$withval
# )
AC_DEFINE(LINK_SIZE, 2, [The value of LINK_SIZE determines the number of bytes used to store dnl
links as offsets within the compiled regex. The default is 2, which allows for dnl
compiled patterns up to 64K long. This covers the vast majority of cases. dnl
However, PCRE can also be compiled to use 3 or 4 bytes instead. This allows for dnl
longer patterns in extreme cases.])


# dnl Handle --with-match_limit=n
#
# AC_ARG_WITH(match-limit,
# [  --with-match-limit=10000000      default limit on internal looping)],
#   MATCH_LIMIT=-DMATCH_LIMIT=$withval
# )
AC_DEFINE(MATCH_LIMIT, 10000000, [The value of MATCH_LIMIT determines the default number of times the match() dnl
function can be called during a single execution of pcre_exec(). (There is a dnl
runtime method of setting a different limit.) The limit exists in order to dnl
catch runaway regular expressions that take for ever to determine that they do dnl
not match. The default is set very large so that it does not accidentally catch dnl
legitimate cases.])


AC_SUBST(PCRE_MAJOR)
AC_SUBST(PCRE_MINOR)
AC_SUBST(PCRE_DATE)
AC_SUBST(PCRE_VERSION)

]) # end of MOO_AC_PCRE
