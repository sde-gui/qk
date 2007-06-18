##############################################################################
# MOO_AC_I18N
#
AC_DEFUN([MOO_AC_PO_GSV],[
  AC_REQUIRE([MOO_AC_I18N])
  AC_CONFIG_COMMANDS([po-gsv/Makefile],[sed -e "/POTFILES =/r po-gsv/POTFILES" po-gsv/Makefile.in > po-gsv/Makefile])
  IT_PO_SUBDIR([po-gsv])
])

AC_DEFUN([MOO_AC_I18N],[
  AC_PROG_INTLTOOL([0.35])

  GETTEXT_PACKAGE=$1
  AC_SUBST(GETTEXT_PACKAGE)
  AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Name of the gettext package.])

  AM_GLIB_GNU_GETTEXT

  MOO_INTLTOOL_INI_RULE='%.ini: %.ini.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po) ; LC_ALL=C $(INTLTOOL_MERGE) -d -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $< [$]@'
  AC_SUBST(MOO_INTLTOOL_INI_RULE)

  MOO_PO_SUBDIRS_RULE='po-subdirs-stamp: $(top_srcdir)/po/POTFILES.in ; potfiles=`cat $(top_srcdir)/po/POTFILES.in` && for potfile in $$potfiles; do mkdir -p $(top_builddir)/`dirname $$potfile`; echo $(top_builddir)/`dirname $$potfile` >> po-subdirs-stamp.tmp; done && mv po-subdirs-stamp.tmp po-subdirs-stamp'
  MOO_PO_SUBDIRS_RULE2='po-subdirs-stamp-2: $(top_srcdir)/po-gsv/POTFILES.in ; potfiles=`cat $(top_srcdir)/po-gsv/POTFILES.in` && for potfile in $$potfiles; do mkdir -p $(top_builddir)/`dirname $$potfile`; echo $(top_builddir)/`dirname $$potfile` >> po-subdirs-stamp.tmp; done && mv po-subdirs-stamp.tmp po-subdirs-stamp'
  AC_SUBST(MOO_PO_SUBDIRS_RULE)
  AC_SUBST(MOO_PO_SUBDIRS_RULE2)
])
