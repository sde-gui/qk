/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* If you are compiling for a system that uses EBCDIC instead of ASCII dnl
   character codes, define this macro as 1. */
#define EBCDIC 0

/* always defined to indicate that i18n is enabled */
/* #undef ENABLE_NLS */

/* Name of the gettext package. */
#define GETTEXT_PACKAGE "moo"

/* Define to 1 if `TIOCGWINSZ' requires <sys/ioctl.h>. */
/* #undef GWINSZ_IN_SYS_IOCTL */

/* Define to 1 if you have the `backtrace' function. */
/* #undef HAVE_BACKTRACE */

/* Define to 1 if you have the `bcopy' function. */
/* #undef HAVE_BCOPY */

/* Define to 1 if you have the `bind_textdomain_codeset' function. */
#define HAVE_BIND_TEXTDOMAIN_CODESET 1

/* Define to 1 if you have the `cfmakeraw' function. */
/* #undef HAVE_CFMAKERAW */

/* Define to 1 if you have the `dcgettext' function. */
#define HAVE_DCGETTEXT 1

/* Define to 1 if you have the <dlfcn.h> header file. */
/* #undef HAVE_DLFCN_H */

/* Define to 1 if you have the <errno.h> header file. */
#define HAVE_ERRNO_H 1

/* Define to 1 if you have the <execinfo.h> header file. */
/* #undef HAVE_EXECINFO_H */

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define to 1 if you have the `getpgid' function. */
/* #undef HAVE_GETPGID */

/* Define to 1 if you have the `getpt' function. */
/* #undef HAVE_GETPT */

/* Define if the GNU gettext() function is already present or preinstalled. */
#define HAVE_GETTEXT 1

/* Define to 1 if you have the `grantpt' function. */
/* #undef HAVE_GRANTPT */

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <io.h> header file. */
#define HAVE_IO_H 1

/* Define to 1 if you have the `kill' function. */
/* #undef HAVE_KILL */

/* Define if your <locale.h> file defines LC_MESSAGES. */
/* #undef HAVE_LC_MESSAGES */

/* Define to 1 if you have the <libutil.h> header file. */
/* #undef HAVE_LIBUTIL_H */

/* Define to 1 if you have the <locale.h> header file. */
#define HAVE_LOCALE_H 1

/* Define to 1 if you have the `memmove' function. */
#define HAVE_MEMMOVE 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `mmap' function. */
/* #undef HAVE_MMAP */

/* Define to 1 if you have the `pipe' function. */
/* #undef HAVE_PIPE */

/* Define to 1 if you have the `poll' function. */
/* #undef HAVE_POLL */

/* Define to 1 if you have the <poll.h> header file. */
/* #undef HAVE_POLL_H */

/* Define to 1 if you have the `posix_openpt' function. */
/* #undef HAVE_POSIX_OPENPT */

/* Define to 1 if you have the `ptsname' function. */
/* #undef HAVE_PTSNAME */

/* Define to 1 if you have the `ptsname_r' function. */
/* #undef HAVE_PTSNAME_R */

/* Define to 1 if you have the `recvmsg' function. */
/* #undef HAVE_RECVMSG */

/* Define to 1 if you have the `signal' function. */
#define HAVE_SIGNAL 1

/* Define to 1 if you have the <signal.h> header file. */
#define HAVE_SIGNAL_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strerror' function. */
#define HAVE_STRERROR 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <stropts.h> header file. */
/* #undef HAVE_STROPTS_H */

/* Define to 1 if you have the <sys/select.h> header file. */
/* #undef HAVE_SYS_SELECT_H */

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/termios.h> header file. */
/* #undef HAVE_SYS_TERMIOS_H */

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <sys/un.h> header file. */
/* #undef HAVE_SYS_UN_H */

/* Define to 1 if you have the <sys/utsname.h> header file. */
/* #undef HAVE_SYS_UTSNAME_H */

/* Define to 1 if you have the <sys/wait.h> header file. */
/* #undef HAVE_SYS_WAIT_H */

/* Define to 1 if you have the <termios.h> header file. */
/* #undef HAVE_TERMIOS_H */

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the `unlink' function. */
#define HAVE_UNLINK 1

/* Define to 1 if you have the `unlockpt' function. */
/* #undef HAVE_UNLOCKPT */

/* Define to 1 if you have the <wchar.h> header file. */
#define HAVE_WCHAR_H 1

/* Define if xmlNode structure has 'line' member */
#define HAVE_XMLNODE_LINE 1

/* Define to 1 if you have the `xmlParseFile' function. */
/* #undef HAVE_XMLPARSEFILE */

/* Define to 1 if you have the `xmlReadFile' function. */
/* #undef HAVE_XMLREADFILE */

/* Define to 1 if you have the `_pipe' function. */
#define HAVE__PIPE 1

/* The value of LINK_SIZE determines the number of bytes used to store dnl
   links as offsets within the compiled regex. The default is 2, which allows
   for dnl compiled patterns up to 64K long. This covers the vast majority of
   cases. dnl However, PCRE can also be compiled to use 3 or 4 bytes instead.
   This allows for dnl longer patterns in extreme cases. */
#define LINK_SIZE 2

/* The value of MATCH_LIMIT determines the default number of times the match()
   dnl function can be called during a single execution of pcre_exec(). (There
   is a dnl runtime method of setting a different limit.) The limit exists in
   order to dnl catch runaway regular expressions that take for ever to
   determine that they do dnl not match. The default is set very large so that
   it does not accidentally catch dnl legitimate cases. */
#define MATCH_LIMIT 10000000

/* The above limit applies to all calls of match(), whether or not they
   increase the recursion depth. In some environments it is desirable to limit
   the depth of recursive calls of match() more strictly, in order to restrict
   the maximum amount of stack (or heap, if NO_RECURSE is defined) that is
   used. The value of MATCH_LIMIT_RECURSION applies only to recursive calls of
   match(). To have any useful effect, it must be less than the value of
   MATCH_LIMIT. There is a runtime method for setting a different limit. On
   systems that support it, "configure" can be used to override this default
   default. */
#define MATCH_LIMIT_RECURSION MATCH_LIMIT

/* MAX_DUPLENGTH */
#define MAX_DUPLENGTH 30000

/* MAX_NAME_COUNT */
#define MAX_NAME_COUNT 10000

/* These three limits are parameterized just in case anybody ever wants to
   change them. Care must be taken if they are increased, because they guard
   against integer overflow caused by enormously large patterns. */
#define MAX_NAME_SIZE 32

/* build mooapp */
#define MOO_BUILD_APP 

/* build mooedit */
#define MOO_BUILD_EDIT 

/* MOO_BUILD_PCRE - build pcre library */
#define MOO_BUILD_PCRE 

/* MOO_BUILD_PYTHON_MODULE */
#define MOO_BUILD_PYTHON_MODULE 1

/* build mooterm */
/* #undef MOO_BUILD_TERM */

/* build mooutils */
#define MOO_BUILD_UTILS 

/* enable relocation */
#define MOO_ENABLE_RELOCATION 1

/* bsd */
/* #undef MOO_OS_BSD */

/* cygwin */
/* #undef MOO_OS_CYGWIN */

/* darwin */
/* #undef MOO_OS_DARWIN */

/* mingw */
#define MOO_OS_MINGW 1

/* unix */
/* #undef MOO_OS_UNIX */

/* package name */
#define MOO_PACKAGE_NAME "moo"

/* MOO_USE_PYGTK */
#define MOO_USE_PYGTK 1

/* MOO_USE_PYTHON */
#define MOO_USE_PYTHON 1

/* use xdgmime */
/* #undef MOO_USE_XDGMIME */

/* use libxml */
#define MOO_USE_XML 1

/* "libmoo version" */
#define MOO_VERSION "0.7.97"

/* "libmoo major version" */
#define MOO_VERSION_MAJOR 0

/* "libmoo micro version" */
#define MOO_VERSION_MICRO 97

/* "libmoo minor version" */
#define MOO_VERSION_MINOR 7

/* The value of NEWLINE determines the newline character used in pcre */
#define NEWLINE '\n'

/* Name of package */
#define PACKAGE "medit"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "muntyan@tamu.edu"

/* Define to the full name of this package. */
#define PACKAGE_NAME "medit"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "medit 0.7.97"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "medit"

/* Define to the version of this package. */
#define PACKAGE_VERSION "0.7.97"

/* POSIX_MALLOC_THRESHOLD */
#define POSIX_MALLOC_THRESHOLD 10

/* Define as the return type of signal handlers (`int' or `void'). */
#define RETSIGTYPE void

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* SUPPORT_UCP */
#define SUPPORT_UCP 

/* SUPPORT_UTF8 */
#define SUPPORT_UTF8 

/* Version number of package */
#define VERSION "0.7.97"

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `int' if <sys/types.h> does not define. */
/* #undef pid_t */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */
