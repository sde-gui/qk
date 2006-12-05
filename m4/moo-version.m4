##############################################################################
# MOO_DEFINE_VERSIONS()
#
AC_DEFUN([MOO_DEFINE_VERSIONS],[
    MOO_VERSION=\"moo_version\"
    MOO_VERSION_UNQUOTED=moo_version
    MOO_VERSION_MAJOR=moo_major_version
    MOO_VERSION_MINOR=moo_minor_version
    MOO_VERSION_MICRO=moo_micro_version

    AC_SUBST(MOO_VERSION)
    AC_SUBST(MOO_VERSION_UNQUOTED)
    AC_SUBST(MOO_VERSION_MAJOR)
    AC_SUBST(MOO_VERSION_MINOR)
    AC_SUBST(MOO_VERSION_MICRO)

    MOO_MODULE_VERSION=\"moo_module_major_version.moo_module_minor_version\"
    MOO_MODULE_VERSION_MAJOR=moo_module_major_version
    MOO_MODULE_VERSION_MINOR=moo_module_minor_version
    AC_SUBST(MOO_MODULE_VERSION_MAJOR)
    AC_SUBST(MOO_MODULE_VERSION_MINOR)

    AC_DEFINE(MOO_VERSION, ["moo_version"], "libmoo version")
    AC_DEFINE(MOO_VERSION_MAJOR, [moo_major_version], "libmoo major version")
    AC_DEFINE(MOO_VERSION_MINOR, [moo_minor_version], "libmoo minor version")
    AC_DEFINE(MOO_VERSION_MICRO, [moo_micro_version], "libmoo micro version")
    AC_DEFINE(MOO_MODULE_VERSION_MAJOR, [moo_module_major_version], "libmoo module system major version")
    AC_DEFINE(MOO_MODULE_VERSION_MINOR, [moo_module_minor_version], "libmoo module system major version")
])
