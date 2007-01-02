AC_DEFUN_ONCE([_MOO_AC_CONFIGARGS_H],[
moo_ac_configure_args=`echo "$ac_configure_args" | sed 's/^ //; s/\\""\`\$/\\\\&/g'`
cat >configargs.h.tmp <<EOF
static const char configure_args@<:@@:>@ = "$moo_ac_configure_args";
EOF
cmp -s configargs.h configargs.h.tmp || mv configargs.h.tmp configargs.h
AC_DEFINE(HAVE_CONFIGARGS_H, 1, [configargs.h is created])
])

##############################################################################
# MOO_AC_FLAGS(moo_top_dir)
#
AC_DEFUN_ONCE([MOO_AC_FLAGS],[
  AC_REQUIRE([MOO_AC_CHECK_OS])
  AC_REQUIRE([MOO_AC_SET_DIRS])

  MOO_AC_FUNCS
  MOO_PKG_CHECK_GTK_VERSIONS
  MOO_AC_DEBUG
  MOO_AC_FAM
  MOO_AC_XML
  MOO_AC_XDGMIME
  MOO_AC_PCRE
  MOO_AC_PYTHON
  dnl must be called after MOO_AC_PYTHON
  MOO_AC_LIB

  moo_top_src_dir=`cd $srcdir && pwd`
  MOO_CFLAGS="$MOO_CFLAGS -I"$moo_top_src_dir/$1" $GTK_CFLAGS $MOO_PCRE_CFLAGS -DXDG_PREFIX=_moo_edit_xdg -DG_LOG_DOMAIN=\\\"Moo\\\""
  MOO_LIBS="$MOO_LIBS $GTK_LIBS $GTHREAD_LIBS $MOO_PCRE_LIBS"

  if test x$MOO_USE_FAM = xyes; then
    MOO_CFLAGS="$MOO_CFLAGS $MOO_FAM_CFLAGS"
    MOO_LIBS="$MOO_LIBS $MOO_FAM_LIBS"
  fi

  MOO_CFLAGS="$MOO_CFLAGS -DMOO_DATA_DIR=\\\"${MOO_DATA_DIR}\\\" -DMOO_LIB_DIR=\\\"${MOO_LIB_DIR}\\\""

  if test x$MOO_USE_GTKHTML = xyes; then
    MOO_CFLAGS="$MOO_CFLAGS $GTKHTML_CFLAGS"
    MOO_LIBS="$MOO_LIBS $GTKHTML_LIBS"
  fi

  ################################################################################
  #  MooEdit stuff
  #
  if test "x$build_mooedit" != "xno"; then
    MOO_CFLAGS="$MOO_CFLAGS $XML_CFLAGS"
    MOO_LIBS="$MOO_LIBS $XML_LIBS"
  fi

  AC_SUBST(MOO_CFLAGS)
  AC_SUBST(MOO_LIBS)

  _MOO_AC_CONFIGARGS_H

  MOO_INI_IN_IN_RULE='%.ini.desktop.in: %.ini.desktop.in.in $(top_builddir)/config.status ; cd $(top_builddir) && $(SHELL) ./config.status --file=$(subdir)/[$]@'
  MOO_INI_IN_RULE='%.ini: %.ini.in $(top_builddir)/config.status ; cd $(top_builddir) && $(SHELL) ./config.status --file=$(subdir)/[$]@'
  MOO_WIN32_RC_RULE='%.res: %.rc.in $(top_builddir)/config.status ; cd $(top_builddir) && $(SHELL) ./config.status --file=$(subdir)/[$]*.rc && cd $(subdir) && $(WINDRES) -i [$]*.rc --input-format=rc -o [$]@ -O coff && rm [$]*.rc'
  AC_SUBST(MOO_INI_IN_IN_RULE)
  AC_SUBST(MOO_INI_IN_RULE)
  AC_SUBST(MOO_WIN32_RC_RULE)
])
