##############################################################################
# MOO_AC_CHECK_XML_STUFF(action-if-found,action-if-not-found)
# checks whether libxml2 is available, checks some functions and structures
#
AC_DEFUN([MOO_AC_CHECK_XML_STUFF],[
    PKG_CHECK_MODULES(XML,libxml-2.0,[
        save_CPPFLAGS="$CPPFLAGS"
        CPPFLAGS="$CPPFLAGS $XML_CFLAGS"
        save_CFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS $XML_CFLAGS"
        save_LDFLAGS="$LDFLAGS"
        LDFLAGS="$LDFLAGS $XML_LIBS"

        AC_CHECK_FUNCS(xmlReadFile xmlParseFile)

        AC_MSG_CHECKING([for xmlNode.line])
        AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
        #include <libxml/parser.h>
        #include <libxml/tree.h>
        int main ()
        {
            xmlNode *node;
            int line;
            line = node->line;
            return 0;
        }]])],[
            AC_MSG_RESULT(present)
            AC_DEFINE(HAVE_XMLNODE_LINE,1,[Define if xmlNode structure has 'line' member])
        ],[
            AC_MSG_RESULT(not present)
        ])

        LDFLAGS="$save_LDFLAGS"
        CFLAGS="$save_CFLAGS"
        CPPFLAGS="$save_CPPFLAGS"

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
AC_DEFUN([MOO_AC_XML],[
    AC_REQUIRE([MOO_AC_CHECK_OS])

    MOO_USE_XML="no"

    AC_ARG_WITH([xml],
        AC_HELP_STRING([--with-xml], [whether to use libxml2 (default: use if present)]),
        [MOO_USE_XML=$withval],
        [MOO_USE_XML=auto]
    )

    if test x$MOO_OS_CYGWIN != "xyes"; then
        if test x$MOO_USE_XML != "xno"; then
            MOO_AC_CHECK_XML_STUFF([moo_found_xml=yes],[moo_found_xml=no])
        fi

        if test x$MOO_USE_XML = "xyes" -a x$moo_found_xml = "xno"; then
            AC_MSG_ERROR([libxml2 library not found])
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
