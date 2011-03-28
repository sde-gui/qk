AC_DEFUN([_MOO_AC_CHECK_C_COMPILER_OPTIONS],[
  AC_LANG_PUSH([C])
  for opt in $1; do
    save_CFLAGS="$CFLAGS"
    CFLAGS="$CFLAGS $opt"
    if test "x$MOO_DEV_MODE" = "xyes"; then
      CFLAGS="-Werror $CFLAGS"
    fi
    AC_TRY_COMPILE([],[],[MOO_CFLAGS="$MOO_CFLAGS $opt"],[:])
    CFLAGS="$save_CFLAGS"
  done
  AC_LANG_POP([C])
])

AC_DEFUN([_MOO_AC_CHECK_CXX_COMPILER_OPTIONS],[
  AC_LANG_PUSH([C++])
  for opt in $1; do
    save_CXXFLAGS="$CXXFLAGS"
    CXXFLAGS="$CXXFLAGS $opt"
    if test "x$MOO_DEV_MODE" = "xyes"; then
      CXXFLAGS="-Werror $CXXFLAGS"
    fi
    AC_TRY_COMPILE([],[],[MOO_CXXFLAGS="$MOO_CXXFLAGS $opt"],[:])
    CXXFLAGS="$save_CXXFLAGS"
  done
  AC_LANG_POP([C++])
])

# _MOO_AC_CHECK_COMPILER_OPTIONS(options)
AC_DEFUN([_MOO_AC_CHECK_COMPILER_OPTIONS],[
  _MOO_AC_CHECK_C_COMPILER_OPTIONS([$1])
  _MOO_AC_CHECK_CXX_COMPILER_OPTIONS([$1])
])

AC_DEFUN([MOO_COMPILER],[
# icc pretends to be gcc or configure thinks it's gcc, but icc doesn't
# error on unknown options, so just don't try gcc options with icc
MOO_ICC=false
MOO_GCC=false
if test "$CC" = "icc"; then
  MOO_ICC=true
elif test "x$GCC" = "xyes"; then
  MOO_GCC=true
fi
])

##############################################################################
# MOO_AC_DEBUG()
#
AC_DEFUN_ONCE([MOO_AC_DEBUG],[

MOO_DEBUG_ENABLED="no"

AC_ARG_ENABLE(debug,
  AC_HELP_STRING([--enable-debug],[enable debug options (default = NO)]),[
  if test "x$enable_debug" = "xno"; then
    MOO_DEBUG_ENABLED="no"
  else
    MOO_DEBUG_ENABLED="yes"
  fi
  ],[
  MOO_DEBUG_ENABLED="no"
])
AM_CONDITIONAL(MOO_DEBUG_ENABLED, test x$MOO_DEBUG_ENABLED = "xyes")

AC_ARG_ENABLE(dev-mode,
  AC_HELP_STRING([--enable-dev-mode],[dev-mode (default = NO, unless --enable-debug is used)]),[
    if test "x$enable_dev_mode" = "xno"; then
      MOO_DEV_MODE="no"
    else
      MOO_DEV_MODE="yes"
    fi
  ],[
  MOO_DEV_MODE="$MOO_DEBUG_ENABLED"
])
AM_CONDITIONAL(MOO_DEV_MODE, test x$MOO_DEV_MODE = "xyes")

MOO_COMPILER

_MOO_AC_CHECK_COMPILER_OPTIONS([dnl
-Wall -Wextra -fexceptions -fno-strict-aliasing -fno-strict-overflow dnl
-Wno-missing-field-initializers -Wno-overlength-strings dnl
-Wno-format-y2k -Wno-overlength-strings dnl
])
_MOO_AC_CHECK_C_COMPILER_OPTIONS([dnl
-Wno-missing-declarations dnl
])
_MOO_AC_CHECK_CXX_COMPILER_OPTIONS([dnl
-std=c++98 -fno-rtti dnl
])

if test "x$MOO_DEBUG_ENABLED" = "xyes"; then
  _MOO_AC_CHECK_COMPILER_OPTIONS([-ftrapv])
else
  _MOO_AC_CHECK_CXX_COMPILER_OPTIONS([-fno-enforce-eh-specs])
fi

if test "x$MOO_DEV_MODE" = "xyes"; then
  if $MOO_GCC; then
    MOO_CFLAGS="$MOO_CFLAGS -Werror"
    MOO_CXXFLAGS="$MOO_CXXFLAGS -Werror"
  fi
  _MOO_AC_CHECK_COMPILER_OPTIONS([dnl
-Wpointer-arith -Wcast-align -Wsign-compare -Wreturn-type dnl
-Wwrite-strings -Wmissing-format-attribute dnl
-Wdisabled-optimization -Wendif-labels dnl
-Wvla -Winit-self dnl
])
  # -Wlogical-op triggers warning in strchr() when compiled with optimizations
  if test "x$MOO_DEBUG_ENABLED" = "xyes"; then
    _MOO_AC_CHECK_COMPILER_OPTIONS([-Wlogical-op])
  else
    _MOO_AC_CHECK_COMPILER_OPTIONS([-Wuninitialized])
  fi
  _MOO_AC_CHECK_C_COMPILER_OPTIONS([dnl
-Wmissing-prototypes -Wnested-externs -Wnolong-long dnl
])
  _MOO_AC_CHECK_CXX_COMPILER_OPTIONS([dnl
-fno-nonansi-builtins -fno-gnu-keywords dnl
-Wctor-dtor-privacy -Wabi -Wstrict-null-sentinel dnl
-Woverloaded-virtual -Wsign-promo -Wnon-virtual-dtor dnl
-Wno-long-long dnl
])
  MOO_CPPFLAGS="$MOO_CPPFLAGS -DG_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED"
fi

# m4_foreach([wname],[unused, sign-compare, write-strings],[dnl
# m4_define([_moo_WNAME],[MOO_W_NO_[]m4_bpatsubst(m4_toupper(wname),-,_)])
# _moo_WNAME=
# _MOO_AC_CHECK_COMPILER_OPTIONS(_moo_WNAME,[-Wno-wname])
# AC_SUBST(_moo_WNAME)
# m4_undefine([_moo_WNAME])
# ])

if test "x$MOO_DEBUG_ENABLED" = "xyes"; then
MOO_CPPFLAGS="$MOO_CPPFLAGS -DENABLE_DEBUG -DENABLE_PROFILE -DG_ENABLE_DEBUG dnl
-DG_ENABLE_PROFILE -DMOO_DEBUG -DDEBUG"
else
MOO_CPPFLAGS="$MOO_CPPFLAGS -DNDEBUG=1 -DG_DISABLE_CAST_CHECKS -DG_DISABLE_ASSERT"
fi
])

##############################################################################
# MOO_AC_SET_DIRS
#
AC_DEFUN_ONCE([MOO_AC_SET_DIRS],[
  if test "x$MOO_PACKAGE_NAME" = x; then
    AC_MSG_ERROR([MOO_PACKAGE_NAME not set])
  fi

  AC_SUBST(MOO_PACKAGE_NAME)
  AC_DEFINE_UNQUOTED([MOO_PACKAGE_NAME], "$MOO_PACKAGE_NAME", [data goes into /usr/share/$MOO_PACKAGE_NAME, etc.])

  AC_SUBST(MOO_DATA_DIR, "${datadir}/$MOO_PACKAGE_NAME")
  AC_SUBST(MOO_LIB_DIR, "${libdir}/$MOO_PACKAGE_NAME")

  AC_SUBST(MOO_DOC_DIR, "${datadir}/doc/$MOO_PACKAGE_NAME")
  AC_SUBST(MOO_HELP_DIR, "${MOO_DOC_DIR}/help")

  AC_SUBST(MOO_TEXT_LANG_FILES_DIR, "${MOO_DATA_DIR}/language-specs")

  AC_DEFINE_UNQUOTED([MOO_PREFS_XML_FILE_NAME], "$MOO_PREFS_XML_FILE_NAME", [prefs.xml])
  AC_DEFINE_UNQUOTED([MOO_STATE_XML_FILE_NAME], "$MOO_STATE_XML_FILE_NAME", [state.xml])
  AC_DEFINE_UNQUOTED([MOO_SESSION_XML_FILE_NAME], "$MOO_SESSION_XML_FILE_NAME", [session.xml])
  AC_DEFINE_UNQUOTED([MOO_NAMED_SESSION_XML_FILE_NAME], "$MOO_NAMED_SESSION_XML_FILE_NAME", [session-%s.xml])

  AC_SUBST(MOO_PYTHON_PLUGIN_DIR, "${MOO_DATA_DIR}/plugins")
  AC_SUBST(MOO_PYTHON_LIB_DIR, "${MOO_DATA_DIR}/python")
])

##############################################################################
# _MOO_AC_CHECK_FAM(action-if-found,action-if-not-found)
#
AC_DEFUN_ONCE([_MOO_AC_CHECK_FAM],[
    moo_ac_save_CFLAGS="$CFLAGS"
    moo_ac_save_LIBS="$LIBS"

    if test x$FAM_LIBS = x; then
        FAM_LIBS=-lfam
    fi

    CFLAGS="$CFLAGS $FAM_CFLAGS"
    LIBS="$LIBS $FAM_LIBS"

    AC_CHECK_HEADERS(fam.h,[
        AC_CHECK_FUNCS([FAMMonitorDirectory FAMOpen],[fam_found=yes],[fam_found=no])
    ],[fam_found=no])

    if test x$fam_found != xno; then
        AC_SUBST(FAM_CFLAGS)
        AC_SUBST(FAM_LIBS)

        AC_MSG_CHECKING(for FAM_CFLAGS)
        if test -z $FAM_CFLAGS; then
            AC_MSG_RESULT(None)
        else
            AC_MSG_RESULT($FAM_CFLAGS)
        fi

        AC_MSG_CHECKING(for FAM_LIBS)
        if test -z $FAM_LIBS; then
            AC_MSG_RESULT(None)
        else
            AC_MSG_RESULT($FAM_LIBS)
        fi

        AC_CHECK_DECL([FAMNoExists],[
          AC_DEFINE(HAVE_FAMNOEXISTS, 1, [fam.h has FAMNoExists defined])
          AC_DEFINE(MOO_USE_GAMIN, 1, [whether libfam is provided by gamin])
        ],[],[#include <fam.h>])

        MOO_FAM_CFLAGS="$FAM_CFLAGS"
        MOO_FAM_LIBS="$FAM_LIBS"
        ifelse([$1], , :, [$1])
    else
        unset FAM_CFLAGS
        unset FAM_LIBS
        MOO_FAM_LIBS=
        MOO_FAM_CFLAGS=
        ifelse([$2], , [AC_MSG_ERROR(libfam not found)], [$2])
    fi

    AC_SUBST(MOO_FAM_CFLAGS)
    AC_SUBST(MOO_FAM_LIBS)
    CFLAGS="$moo_ac_save_CFLAGS"
    LIBS="$moo_ac_save_LIBS"
])

AC_DEFUN_ONCE([MOO_AC_FAM],[
  if $MOO_OS_UNIX; then
    AC_ARG_WITH([fam], AC_HELP_STRING([--with-fam], [whether to use fam or gamin for monitoring files in the editor (default = NO)]), [
      if test x$with_fam = "xyes"; then
        MOO_USE_FAM="yes"
      else
        MOO_USE_FAM="no"
      fi
    ],[
      MOO_USE_FAM="no"
    ])

    if test "$MOO_USE_FAM" = "yes"; then
      _MOO_AC_CHECK_FAM([moo_has_fam=yes],[moo_has_fam=no])
      if test x$moo_has_fam = xyes; then
        MOO_USE_FAM="yes"
        AC_DEFINE(MOO_USE_FAM, 1, [use libfam for monitoring files])
      else
        AC_MSG_ERROR([FAM or gamin not found.])
      fi
    fi
  fi

  AM_CONDITIONAL(MOO_USE_FAM, test x$MOO_USE_FAM = "xyes")
])

##############################################################################
# MOO_AC_FLAGS(moo_top_dir)
#
AC_DEFUN_ONCE([MOO_AC_FLAGS],[
  AC_REQUIRE([MOO_AC_CHECK_OS])
  AC_REQUIRE([MOO_AC_SET_DIRS])

  MOO_PKG_CHECK_GTK_VERSIONS
  MOO_AC_DEBUG
  MOO_AC_FAM

  AC_CHECK_FUNCS_ONCE(getc_unlocked)
  AC_CHECK_HEADERS(unistd.h sys/utsname.h signal.h)

  if $MOO_OS_WIN32; then
    AC_DEFINE(HAVE_MMAP, 1, [Fake mmap on win32])
  else
    AC_CHECK_FUNCS(mmap)
  fi

  AC_DEFINE(MOO_COMPILATION, 1, [must be 1])

  moo_top_src_dir=`cd $srcdir && pwd`
  MOO_CFLAGS="$MOO_CFLAGS $GTK_CFLAGS"
  MOO_CXXFLAGS="$MOO_CXXFLAGS $GTK_CFLAGS"
  MOO_CPPFLAGS="$MOO_CPPFLAGS -I$moo_top_src_dir/moo -DXDG_PREFIX=_moo_edit_xdg -DG_LOG_DOMAIN=\\\"Moo\\\""
  MOO_LIBS="$MOO_LIBS $GTK_LIBS $GTHREAD_LIBS $LIBM"

  if $GDK_X11; then
    _moo_x_pkgs=
    m4_foreach([_pkg_],[x11, xext, xrender, ice, sm],[
      PKG_CHECK_EXISTS(_pkg_,[_moo_x_pkgs="$_moo_x_pkgs _pkg_"],[:])
    ])
    if test -n "$_moo_x_pkgs"; then
      PKG_CHECK_MODULES(X,[$_moo_x_pkgs])
      MOO_CFLAGS="$MOO_CFLAGS $X_CFLAGS"
      MOO_CXXFLAGS="$MOO_CXXFLAGS $X_CFLAGS"
      MOO_LIBS="$MOO_LIBS $X_LIBS"
    fi
  fi

  if $MOO_OS_WIN32; then
    MOO_CPPFLAGS="$MOO_CPPFLAGS -DUNICODE -D_UNICODE -DSTRICT -DWIN32_LEAN_AND_MEAN -I$moo_top_src_dir/moo/mooutils/moowin32/mingw"
  fi

  if test x$MOO_USE_FAM = xyes; then
    MOO_CFLAGS="$MOO_CFLAGS $MOO_FAM_CFLAGS"
    MOO_CXXFLAGS="$MOO_CXXFLAGS $MOO_FAM_CFLAGS"
    MOO_LIBS="$MOO_LIBS $MOO_FAM_LIBS"
  fi

  if $MOO_OS_UNIX; then
    MOO_CPPFLAGS="$MOO_CPPFLAGS -DMOO_DATA_DIR=\\\"${MOO_DATA_DIR}\\\" -DMOO_LIB_DIR=\\\"${MOO_LIB_DIR}\\\""
    MOO_CPPFLAGS="$MOO_CPPFLAGS -DMOO_LOCALE_DIR=\\\"${localedir}\\\" -DMOO_HELP_DIR=\\\"${MOO_HELP_DIR}\\\""
  fi

  MOO_CFLAGS="$MOO_CFLAGS $XML_CFLAGS"
  MOO_CXXFLAGS="$MOO_CXXFLAGS $XML_CFLAGS"
  MOO_LIBS="$MOO_LIBS $XML_LIBS"

  AC_SUBST(MOO_CPPFLAGS)
  AC_SUBST(MOO_CFLAGS)
  AC_SUBST(MOO_CXXFLAGS)
  AC_SUBST(MOO_LIBS)

#   MOO_INI_IN_IN_RULE='%.ini.desktop.in: %.ini.desktop.in.in $(top_builddir)/config.status ; cd $(top_builddir) && $(SHELL) ./config.status --file=$(subdir)/[$]@'
#   MOO_INI_IN_RULE='%.ini: %.ini.in $(top_builddir)/config.status ; cd $(top_builddir) && $(SHELL) ./config.status --file=$(subdir)/[$]@'
#   MOO_WIN32_RC_RULE='%.res: %.rc.in $(top_builddir)/config.status ; cd $(top_builddir) && $(SHELL) ./config.status --file=$(subdir)/[$]*.rc && cd $(subdir) && $(WINDRES) -i [$]*.rc --input-format=rc -o [$]@ -O coff && rm [$]*.rc'
#   AC_SUBST(MOO_INI_IN_IN_RULE)
#   AC_SUBST(MOO_INI_IN_RULE)
#   AC_SUBST(MOO_WIN32_RC_RULE)

#   MOO_XML2H='$(top_srcdir)/moo/mooutils/xml2h.sh'
#   MOO_GLADE_SUBDIR_RULE='%-glade.h: glade/%.glade $(MOO_XML2H) ; $(SHELL) $(top_srcdir)/moo/mooutils/xml2h.sh `basename "[$]*" | sed -e "s/-/_/"`_glade_xml [$]< > [$]@.tmp && mv [$]@.tmp [$]@'
#   MOO_GLADE_RULE='%-glade.h: %.glade $(MOO_XML2H) ; $(SHELL) $(top_srcdir)/moo/mooutils/xml2h.sh `basename "[$]*" | sed -e "s/-/_/"`_glade_xml [$]< > [$]@.tmp && mv [$]@.tmp [$]@'
#   AC_SUBST(MOO_XML2H)
#   AC_SUBST(MOO_GLADE_SUBDIR_RULE)
#   AC_SUBST(MOO_GLADE_RULE)
])
