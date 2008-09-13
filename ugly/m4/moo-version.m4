##############################################################################
# _MOO_SPLIT_VERSION(NAME,version)
#
AC_DEFUN([_MOO_SPLIT_VERSION],[AC_REQUIRE([LT_AC_PROG_SED])
$1[]_VERSION="$2"
$1[]_MAJOR_VERSION=`echo "$2" | $SED 's/\([[^.]][[^.]]*\).*/\1/'`
$1[]_MINOR_VERSION=`echo "$2" | $SED 's/[[^.]][[^.]]*.\([[^.]][[^.]]*\).*/\1/'`
$1[]_MICRO_VERSION=`echo "$2" | $SED 's/[[^.]][[^.]]*.[[^.]][[^.]]*.\(.*\)/\1/'`
])

##############################################################################
# MOO_DEFINE_VERSIONS(pkg,version,detail)
#
AC_DEFUN([MOO_DEFINE_VERSIONS],[
_MOO_SPLIT_VERSION([m4_toupper($1)], [$2])
m4_toupper($1)_VERSION=\"$2[]$3\"
m4_toupper($1)_VERSION_UNQUOTED=$2[]$3
AC_SUBST(m4_toupper($1)_VERSION)
AC_SUBST(m4_toupper($1)_VERSION_UNQUOTED)
AC_SUBST(m4_toupper($1)_MAJOR_VERSION)
AC_SUBST(m4_toupper($1)_MINOR_VERSION)
AC_SUBST(m4_toupper($1)_MICRO_VERSION)
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
