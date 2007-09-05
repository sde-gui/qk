AC_DEFUN([MOO_LUA],[
  AC_REQUIRE([MOO_AC_CHECK_OS])

  MOO_LUA_CFLAGS=

  if test "x$MOO_OS_DARWIN" = "xyes"; then
    MOO_LUA_CFLAGS="-DLUA_USE_MACOSX"
  elif test "x$MOO_OS_MINGW" != "xyes"; then
    MOO_LUA_CFLAGS="-DLUA_USE_POSIX -DLUA_USE_DLOPEN"
  fi

  # for lua-ex
  _moo_ac_have_posix_spawn=no
  AC_CHECK_FUNCS(posix_spawn,[_moo_ac_have_posix_spawn=yes])
  AM_CONDITIONAL(HAVE_POSIX_SPAWN,[test $_moo_ac_have_posix_spawn = yes])

  AC_SUBST([MOO_LUA_CFLAGS])
])
