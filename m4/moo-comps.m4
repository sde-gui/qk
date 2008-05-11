##############################################################################
# MOO_COMPONENTS(default-yes,default-no)
#
AC_DEFUN([MOO_COMPONENTS],[
  AC_REQUIRE([MOO_AC_CHECK_OS])

  m4_foreach([comp], [utils, edit, term, app],
             [build_moo[]comp=true])
  m4_foreach([comp], $2,
             [build_moo[]comp=false])

  m4_foreach([comp], [$1],
  [AC_ARG_WITH([moo[]comp],
    AC_HELP_STRING([--without-moo[]comp], [disable moo[]comp component (default = NO)]),
    [if test "x$withval" = "xyes"; then build_moo[]comp=true; else build_moo[]comp=false; fi])
  ])

  m4_foreach([comp], [$2],
  [AC_ARG_WITH([moo[]comp],
    AC_HELP_STRING([--with-moo[]comp], [enable moo[]comp component (default = NO)]),
    [if test "x$withval" = "xyes"; then build_moo[]comp=true; else build_moo[]comp=false; fi])])

  if test "x$MOO_OS_CYGWIN" = "xyes"; then
    build_mooutils=false
    build_mooedit=false
    build_mooapp=false
    build_mooterm=true
  fi

  if $build_mooapp; then
    build_mooedit=true
  fi
  if $build_mooedit; then
    build_mooutils=true
  fi
  if test "x$MOO_OS_CYGWIN" != "xyes"; then
    $build_mooterm && build_mooutils=true
  fi

  build_lua=$build_mooedit
  MOO_BUILD_COMPS=

  AM_CONDITIONAL(MOO_BUILD_LUA, [$build_lua])
  MOO_BUILD_LUA=false
  if $build_lua; then
    AC_DEFINE(MOO_BUILD_LUA, [1], [build lua])
    MOO_BUILD_LUA=true
    MOO_BUILD_COMPS="lua $MOO_BUILD_COMPS"
    MOO_LUA_ENABLED_DEFINE=["#define MOO_LUA_ENABLED 1"]
  else
    MOO_LUA_ENABLED_DEFINE=["#undef MOO_LUA_ENABLED"]
  fi
  AC_SUBST(MOO_LUA_ENABLED_DEFINE)

  m4_foreach([comp], [utils, edit, term, app],[
    AM_CONDITIONAL(MOO_BUILD_[]m4_toupper(comp), $build_moo[]comp)
    MOO_BUILD_[]m4_toupper(comp)=false
    if $build_moo[]comp; then
      AC_DEFINE(MOO_BUILD_[]m4_toupper(comp), [1], [build moo]comp)
      MOO_BUILD_[]m4_toupper(comp)=true
      MOO_BUILD_COMPS="moo[]comp $MOO_BUILD_COMPS"
      MOO_[]m4_toupper(comp)_ENABLED_DEFINE=["#define MOO_]m4_toupper(comp)[_ENABLED 1"]
    else
      MOO_[]m4_toupper(comp)_ENABLED_DEFINE=["#undef MOO_]m4_toupper(comp)[_ENABLED"]
    fi
    AC_SUBST(MOO_[]m4_toupper(comp)_ENABLED_DEFINE)
  ])

  if test "x$MOO_OS_BSD" = "xyes" -a "x$MOO_OS_DARWIN" != "xyes"; then
    $build_mooterm && MOO_LIBS="-lutil $MOO_LIBS"
  fi

  AC_ARG_ENABLE(ctags-plugin,
    AC_HELP_STRING(--enable-ctags-plugin, [enable ctags plugin (default = YES)]),
    [:],[enable_ctags_plugin=yes])

  if test "x$MOO_BUILD_CTAGS" != xno -a "x$MOO_OS_CYGWIN" != "xyes" -a \
          "x$MOO_OS_MINGW" != "xyes" -a "x$enable_ctags_plugin" = "xyes"; then
    MOO_BUILD_CTAGS=yes
    AC_DEFINE(MOO_BUILD_CTAGS, [1], [build ctags plugin])
  else
    MOO_BUILD_CTAGS=no
  fi
  AM_CONDITIONAL(MOO_BUILD_CTAGS, test "x$MOO_BUILD_CTAGS" = xyes)

  AC_ARG_ENABLE(project,
    AC_HELP_STRING(--enable-project, [enable project plugin (default = NO)]),
    [:],[enable_project=no])
  AM_CONDITIONAL(MOO_ENABLE_PROJECT, test "x$enable_project" = xyes)
])
