##############################################################################
# MOO_AC_I18N
#
AC_DEFUN([MOO_AC_I18N],[
    AC_PROG_INTLTOOL([0.29])

    GETTEXT_PACKAGE=moo
    AC_SUBST(GETTEXT_PACKAGE)
    AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Name of the gettext package.])

    ALL_LINGUAS="$1"
    AM_GLIB_GNU_GETTEXT

    MOO_INTLTOOL_INI_RULE='%.ini: %.ini.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po) ; cp $< [$]@.desktop.in && LC_ALL=C $(INTLTOOL_MERGE) -d -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po [$]@.desktop.in [$]@ && rm [$]@.desktop.in'
    AC_SUBST(MOO_INTLTOOL_INI_RULE)
])
