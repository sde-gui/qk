##############################################################################
# MOO_DEFINE_VERSIONS(pkg,version)
#
AC_DEFUN([MOO_DEFINE_VERSIONS],[
_MOO_SPLIT_VERSION([m4_toupper($1)], [$2])
m4_toupper($1)_VERSION=\"$2\"
m4_toupper($1)_VERSION_UNQUOTED=$2
AC_SUBST(m4_toupper($1)_VERSION)
AC_SUBST(m4_toupper($1)_VERSION_UNQUOTED)
AC_SUBST(m4_toupper($1)_MAJOR_VERSION)
AC_SUBST(m4_toupper($1)_MINOR_VERSION)
AC_SUBST(m4_toupper($1)_MICRO_VERSION)
AC_DEFINE(m4_toupper($1)_VERSION, ["$2"], [$1 version])
AC_DEFINE_UNQUOTED(m4_toupper($1)_MAJOR_VERSION, [$[]m4_toupper($1)_MAJOR_VERSION], [$1 major version])
AC_DEFINE_UNQUOTED(m4_toupper($1)_MINOR_VERSION, [$[]m4_toupper($1)_MINOR_VERSION], [$1 minor version])
AC_DEFINE_UNQUOTED(m4_toupper($1)_MICRO_VERSION, [$[]m4_toupper($1)_MICRO_VERSION], [$1 micro version])
]) # MOO_DEFINE_VERSIONS

##############################################################################
# MOO_DEFINE_MODULE_VERSIONS(major,minor)
#
AC_DEFUN([MOO_DEFINE_MODULE_VERSIONS],[
MOO_MODULE_VERSION=\"$1.$2\"
MOO_MODULE_MAJOR_VERSION=$1
MOO_MODULE_MINOR_VERSION=$2
AC_SUBST(MOO_MODULE_VERSION)
AC_SUBST(MOO_MODULE_MAJOR_VERSION)
AC_SUBST(MOO_MODULE_MINOR_VERSION)
AC_DEFINE(MOO_MODULE_MAJOR_VERSION, [$1], [libmoo module system major version])
AC_DEFINE(MOO_MODULE_MINOR_VERSION, [$2], [libmoo module system minor version])
]) # MOO_DEFINE_MODULE_VERSIONS
