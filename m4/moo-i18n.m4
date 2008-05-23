##############################################################################
# MOO_AC_I18N
#
AC_DEFUN_ONCE([MOO_AC_PO_GSV],[
  AC_REQUIRE([MOO_AC_I18N])
  AC_CONFIG_COMMANDS([po-gsv/Makefile],[sed -e "/POTFILES =/r po-gsv/POTFILES" po-gsv/Makefile.in > po-gsv/Makefile])
  IT_PO_SUBDIR([po-gsv])
])

AC_DEFUN_ONCE([MOO_AC_I18N],[
  AC_REQUIRE([MOO_AC_CHECK_OS])
  AC_REQUIRE([MOO_AC_SRCDIR])

  m4_if([$1],[],[
    YOU_FORGOT_TO_CALL_MOO_AC_I18N_WITH_RIGHT_PARAMETERS()
  ])

  _moo_enable_nls=yes

  if test "x$MOO_OS_CYGWIN" = "xyes"; then
    _moo_enable_nls=no
  fi

  m4_if([$2],[no],[
    _moo_enable_nls=no
  ],[
    AC_ARG_ENABLE(nls,
      AC_HELP_STRING(--disable-nls, [do not try to use gettext and friends]),
      [_moo_enable_nls=$enable_nls],[:])
  ])

  GETTEXT_PACKAGE=$1
  AC_SUBST(GETTEXT_PACKAGE)
  AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Name of the gettext package.])

  MOO_INTLTOOL_MERGE='$(moo_srcdir)/mooutils/moo-intltool-merge'

  m4_if([$2],[no],[
    MOO_INTLTOOL_XML_RULE='%.xml: %.xml.in $(MOO_INTLTOOL_MERGE) ; $(MOO_INTLTOOL_MERGE) $< [$]@'
    MOO_INTLTOOL_DESKTOP_RULE='%.desktop: %.desktop.in $(MOO_INTLTOOL_MERGE) ; $(MOO_INTLTOOL_MERGE) $< [$]@'
    MOO_INTLTOOL_INI_RULE='%.ini: %.ini.desktop.in $(MOO_INTLTOOL_MERGE) ; $(MOO_INTLTOOL_MERGE) $< [$]@'
    MOO_PO_SUBDIRS_RULE='po-subdirs-stamp: ; echo dummy > po-subdirs-stamp'
    MOO_PO_SUBDIRS_RULE2='po-subdirs-stamp-2: ; echo dummy > po-subdirs-stamp-2'
  ],[
    if test "x$_moo_enable_nls" = xyes; then
      AC_PROG_INTLTOOL([0.35])
      AM_GLIB_GNU_GETTEXT

      # these two are copied from intltool, need them under different name to use with --disable-nls
      MOO_INTLTOOL_XML_RULE='%.xml: %.xml.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po) ; LC_ALL=C $(INTLTOOL_MERGE) -x -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $< [$]@'
      MOO_INTLTOOL_DESKTOP_RULE='%.desktop: %.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po) ; LC_ALL=C $(INTLTOOL_MERGE) -d -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $< [$]@'
      MOO_INTLTOOL_INI_RULE='%.ini: %.ini.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po) ; LC_ALL=C $(INTLTOOL_MERGE) -d -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $< [$]@'
      MOO_PO_SUBDIRS_RULE='po-subdirs-stamp: $(top_srcdir)/po/POTFILES.in ; (for potfile in `cat $<`; do echo `dirname $$potfile`; done) | uniq > $(@).dirs && for subdir in `cat $(@).dirs`; do echo $$subdir >> $(@).tmp; mkdir -p $(top_builddir)/$$subdir; done && rm $(@).dirs && mv $(@).tmp $(@)'
      MOO_PO_SUBDIRS_RULE2='po-subdirs-stamp-2: $(top_srcdir)/po-gsv/POTFILES.in ; (for potfile in `cat $<`; do echo `dirname $$potfile`; done) | uniq > $(@).dirs && for subdir in `cat $(@).dirs`; do echo $$subdir >> $(@).tmp; mkdir -p $(top_builddir)/$$subdir; done && rm $(@).dirs && mv $(@).tmp $(@)'
    else
      MOO_INTLTOOL_XML_RULE='%.xml: %.xml.in $(MOO_INTLTOOL_MERGE) ; $(MOO_INTLTOOL_MERGE) $< [$]@'
      MOO_INTLTOOL_DESKTOP_RULE='%.desktop: %.desktop.in $(MOO_INTLTOOL_MERGE) ; $(MOO_INTLTOOL_MERGE) $< [$]@'
      MOO_INTLTOOL_INI_RULE='%.ini: %.ini.desktop.in $(MOO_INTLTOOL_MERGE) ; $(MOO_INTLTOOL_MERGE) $< [$]@'
      MOO_PO_SUBDIRS_RULE='po-subdirs-stamp: ; echo dummy > po-subdirs-stamp'
      MOO_PO_SUBDIRS_RULE2='po-subdirs-stamp-2: ; echo dummy > po-subdirs-stamp-2'

      AC_CONFIG_COMMANDS([po/Makefile],[sed -e "/POTFILES =/r po/POTFILES" po/Makefile.in > po/Makefile])
    fi
  ])

  AC_SUBST(MOO_INTLTOOL_MERGE)
  AC_SUBST(MOO_INTLTOOL_DESKTOP_RULE)
  AC_SUBST(MOO_INTLTOOL_XML_RULE)
  AC_SUBST(MOO_PO_SUBDIRS_RULE)
  AC_SUBST(MOO_PO_SUBDIRS_RULE2)
  AC_SUBST(MOO_INTLTOOL_INI_RULE)

  AM_CONDITIONAL(MOO_ENABLE_NLS, [test "x$_moo_enable_nls" = xyes])
])
