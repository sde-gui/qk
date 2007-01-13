##############################################################################
# MOO_AC_CHECK_XML_STUFF(action-if-found,action-if-not-found)
# checks whether libxml2 is available, checks some functions and structures
#
AC_DEFUN_ONCE([MOO_AC_CHECK_XML_STUFF],[
    PKG_CHECK_MODULES(XML,libxml-2.0,[
        _moo_ac_xml_libs=`$PKG_CONFIG --libs-only-l libxml-2.0`
        _moo_ac_xml_ldflags=`$PKG_CONFIG --libs-only-L libxml-2.0`
        moo_ac_save_CPPFLAGS="$CPPFLAGS"
        CPPFLAGS="$CPPFLAGS $XML_CFLAGS"
        moo_ac_save_CFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS $XML_CFLAGS"
        moo_ac_save_LDFLAGS="$LDFLAGS"
        LDFLAGS="$LDFLAGS $_moo_ac_xml_ldflags"
        moo_ac_save_LIBS="$LIBS"
        LIBS="$LIBS $_moo_ac_xml_libs"

        AC_CHECK_FUNCS(xmlReadFile xmlParseFile)
        AC_CHECK_MEMBER([xmlNode.line],
                        [AC_DEFINE(HAVE_XMLNODE_LINE,1,[Define if xmlNode structure has 'line' member])],
                        [],[#include <libxml/tree.h>])

        LIBS="$moo_ac_save_LIBS"
        LDFLAGS="$moo_ac_save_LDFLAGS"
        CFLAGS="$moo_ac_save_CFLAGS"
        CPPFLAGS="$moo_ac_save_CPPFLAGS"

        MOO_XML_PKG_NAME=libxml-2.0

        $1
    ],[
        MOO_XML_PKG_NAME=
        $2
    ])

    AC_SUBST(MOO_XML_PKG_NAME)
])


##############################################################################
# MOO_AC_XML([])
#
AC_DEFUN_ONCE([MOO_AC_XML],[
    AC_REQUIRE([MOO_AC_CHECK_OS])

    MOO_USE_XML="no"

    AC_ARG_WITH([xml],
        AC_HELP_STRING([--with-xml], [whether to use libxml2 (default = YES)]),
        [MOO_USE_XML=$withval],
        [MOO_USE_XML=auto]
    )

    if test x$MOO_OS_CYGWIN != "xyes"; then
        if test x$MOO_USE_XML != "xno"; then
            MOO_AC_CHECK_XML_STUFF([moo_found_xml=yes],[moo_found_xml=no])
        fi

        if test x$MOO_USE_XML = "xyes" -a x$moo_found_xml = "xno"; then
            AC_MSG_ERROR([libxml2 library not found])
        elif test x$MOO_USE_XML != "xno" -a x$moo_found_xml = "xno"; then
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
