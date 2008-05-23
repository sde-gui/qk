##############################################################################
# MOO_AC_XML([])
#
AC_DEFUN_ONCE([MOO_AC_XML],[
  AC_REQUIRE([MOO_AC_CHECK_OS])
  AC_REQUIRE([MOO_COMPONENTS])

  MOO_USE_XML="no"

  AC_ARG_WITH([xml],[
    AC_HELP_STRING([--with-xml], [whether to use libxml2 (default = YES)])
  ],[
    MOO_USE_XML=$withval
  ],[
    MOO_USE_XML=auto
  ])

  if test x$MOO_OS_CYGWIN != "xyes"; then
    if test x$MOO_USE_XML != "xno"; then
      PKG_CHECK_MODULES(XML,[libxml-2.0],[moo_found_xml=yes],[moo_found_xml=no])
    fi

    if test x$MOO_USE_XML = "xyes" -a x$moo_found_xml = "xno"; then
      AC_MSG_ERROR([libxml2 library not found])
    elif test x$MOO_USE_XML != "xno" -a x$moo_found_xml = "xno" -a $MOO_BUILD_EDIT; then
      AC_MSG_WARN([libxml2 library not found, syntax highlighting in the editor will be disabled])
    fi

    if test x$moo_found_xml = "xyes"; then
      MOO_USE_XML=yes
      AC_MSG_NOTICE([compiling with xml support])
      AC_DEFINE(MOO_USE_XML, 1, [use libxml])
    else
      MOO_USE_XML=no
      AC_MSG_NOTICE([compiling without xml support])
    fi
  fi

  AM_CONDITIONAL(MOO_USE_XML, test x$MOO_USE_XML = xyes)
])
