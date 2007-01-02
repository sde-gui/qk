##############################################################################
# MOO_DEFINE_VERSIONS(pkg,major,minor,micro)
#
AC_DEFUN([MOO_DEFINE_VERSIONS],[
m4_toupper($1)_VERSION=\"$2.$3.$4\"
m4_toupper($1)_VERSION_UNQOTED=$2.$3.$4

m4_toupper($1)_VERSION_MAJOR=$2
m4_toupper($1)_VERSION_MINOR=$3
m4_toupper($1)_VERSION_MICRO=$4

AC_SUBST(m4_toupper($1)_VERSION)
AC_SUBST(m4_toupper($1)_VERSION_UNQUOTED)
AC_SUBST(m4_toupper($1)_VERSION_MAJOR)
AC_SUBST(m4_toupper($1)_VERSION_MINOR)
AC_SUBST(m4_toupper($1)_VERSION_MICRO)

AC_DEFINE(m4_toupper($1)_VERSION, ["$2.$3.$4"], "$1 version")
AC_DEFINE(m4_toupper($1)_VERSION_MAJOR, [$2], "$1 major version")
AC_DEFINE(m4_toupper($1)_VERSION_MINOR, [$3], "$1 minor version")
AC_DEFINE(m4_toupper($1)_VERSION_MICRO, [$4], "$1 micro version")
])

##############################################################################
# MOO_DEFINE_MODULE_VERSIONS(major,minor)
#
AC_DEFUN([MOO_DEFINE_MODULE_VERSIONS],[
    MOO_MODULE_VERSION=\"$1.$2\"
    MOO_MODULE_VERSION_MAJOR=$1
    MOO_MODULE_VERSION_MINOR=$2
    AC_SUBST(MOO_MODULE_VERSION)
    AC_SUBST(MOO_MODULE_VERSION_MAJOR)
    AC_SUBST(MOO_MODULE_VERSION_MINOR)
    AC_DEFINE(MOO_MODULE_VERSION_MAJOR, [$1], "libmoo module system major version")
    AC_DEFINE(MOO_MODULE_VERSION_MINOR, [$2], "libmoo module system major version")
])
