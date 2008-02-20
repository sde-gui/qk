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

  AC_PATH_XTRA
  MOO_AC_FUNCS
  MOO_PKG_CHECK_GTK_VERSIONS
  MOO_AC_DEBUG
  MOO_AC_FAM
  MOO_AC_XML
  MOO_AC_PCRE
  MOO_AC_PYTHON
  dnl must be called after MOO_AC_PYTHON
  MOO_AC_LIB

  MOO_ENABLE_GENERATED_FILES="yes"
  AC_ARG_ENABLE(generated-files,
    AC_HELP_STRING([--enable-generated-files],[run update-mime-database and gtk-update-icon-cache on install (default = YES)]),[
    if test "x$enable_generated_files" = "xno"; then
      MOO_ENABLE_GENERATED_FILES="no"
    fi ])
  AM_CONDITIONAL(MOO_ENABLE_GENERATED_FILES, test x$MOO_ENABLE_GENERATED_FILES = "xyes")

  AC_CHECK_LIB(Xrender, XRenderFindFormat,[
    AC_SUBST(RENDER_LIBS, "-lXrender -lXext")
    AC_DEFINE(HAVE_RENDER, 1, [Define if libXrender is available.])
  ],[
    AC_SUBST(RENDER_LIBS, "")
  ],[-lXext])

  AC_DEFINE(MOO_COMPILATION, 1, [must be 1])

  moo_top_src_dir=`cd $srcdir && pwd`
  MOO_CFLAGS="$MOO_CFLAGS -I"$moo_top_src_dir/$1" $GTK_CFLAGS $MOO_PCRE_CFLAGS -DXDG_PREFIX=_moo_edit_xdg -DG_LOG_DOMAIN=\\\"Moo\\\""
  MOO_LIBS="$MOO_LIBS $GTK_LIBS $GTHREAD_LIBS $MOO_PCRE_LIBS -lm"

  if test "x$GLIB_2_14" != xyes; then
    MOO_CFLAGS="-I$moo_top_src_dir/$1/mooutils/newgtk/glib-2.14 $MOO_CFLAGS"
  fi
  if test "x$GLIB_2_12" != xyes; then
    MOO_CFLAGS="-I$moo_top_src_dir/$1/mooutils/newgtk/glib-2.12 $MOO_CFLAGS"
  fi
  if test "x$GLIB_2_8" != xyes; then
    MOO_CFLAGS="-I$moo_top_src_dir/$1/mooutils/newgtk/glib-2.8 $MOO_CFLAGS"
  fi

  if test x$MOO_USE_FAM = xyes; then
    MOO_CFLAGS="$MOO_CFLAGS $MOO_FAM_CFLAGS"
    MOO_LIBS="$MOO_LIBS $MOO_FAM_LIBS"
  fi

  MOO_CFLAGS="$MOO_CFLAGS -DMOO_DATA_DIR=\\\"${MOO_DATA_DIR}\\\" -DMOO_LIB_DIR=\\\"${MOO_LIB_DIR}\\\""

  if test x$MOO_USE_GTKHTML = xyes; then
    MOO_CFLAGS="$MOO_CFLAGS $GTKHTML_CFLAGS"
    MOO_LIBS="$MOO_LIBS $GTKHTML_LIBS"
  fi

  if test "x$MOO_OS_MINGW" = xyes; then
    MOO_CFLAGS="$MOO_CFLAGS -DWIN32_LEAN_AND_MEAN -DUNICODE"
    MOO_WIN32_CFLAGS="-I$moo_top_src_dir/$1/mooutils/moowin32/mingw"
    AC_DEFINE(HAVE_MMAP, [1], [using fake mmap on windows])
  fi

  ################################################################################
  #  MooEdit stuff
  #
  if test "x$build_mooedit" != "xno"; then
    MOO_CFLAGS="$MOO_CFLAGS $XML_CFLAGS"
    MOO_LIBS="$MOO_LIBS $XML_LIBS"
  fi

  AC_SUBST(MOO_WIN32_CFLAGS)
  AC_SUBST(MOO_CFLAGS)
  AC_SUBST(MOO_LIBS)

  _MOO_AC_CONFIGARGS_H

  MOO_INI_IN_IN_RULE='%.ini.desktop.in: %.ini.desktop.in.in $(top_builddir)/config.status ; cd $(top_builddir) && $(SHELL) ./config.status --file=$(subdir)/[$]@'
  MOO_INI_IN_RULE='%.ini: %.ini.in $(top_builddir)/config.status ; cd $(top_builddir) && $(SHELL) ./config.status --file=$(subdir)/[$]@'
  MOO_WIN32_RC_RULE='%.res: %.rc.in $(top_builddir)/config.status ; cd $(top_builddir) && $(SHELL) ./config.status --file=$(subdir)/[$]*.rc && cd $(subdir) && $(WINDRES) -i [$]*.rc --input-format=rc -o [$]@ -O coff && rm [$]*.rc'
  AC_SUBST(MOO_INI_IN_IN_RULE)
  AC_SUBST(MOO_INI_IN_RULE)
  AC_SUBST(MOO_WIN32_RC_RULE)

  MOO_XML2H='$(top_srcdir)/moo/mooutils/xml2h.sh'
  MOO_GLADE_SUBDIR_RULE='%-glade.h: glade/%.glade $(MOO_XML2H) ; $(SHELL) $(top_srcdir)/moo/mooutils/xml2h.sh `basename "[$]*" | sed -e "s/-/_/"`_glade_xml [$]< > [$]@.tmp && mv [$]@.tmp [$]@'
  MOO_GLADE_RULE='%-glade.h: %.glade $(MOO_XML2H) ; $(SHELL) $(top_srcdir)/moo/mooutils/xml2h.sh `basename "[$]*" | sed -e "s/-/_/"`_glade_xml [$]< > [$]@.tmp && mv [$]@.tmp [$]@'
  AC_SUBST(MOO_XML2H)
  AC_SUBST(MOO_GLADE_SUBDIR_RULE)
  AC_SUBST(MOO_GLADE_RULE)
])
