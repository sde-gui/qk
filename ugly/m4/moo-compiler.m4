# _MOO_AC_CHECK_COMPILER_OPTIONS(var,options,lang,verbose)
AC_DEFUN([__MOO_AC_CHECK_COMPILER_OPTIONS],[
  AC_LANG_SAVE
  AC_LANG_PUSH($3)
  for opt in $2; do
    m4_if([$4],[verbose],[AC_MSG_CHECKING(whether $3 compiler accepts $opt)])
    save_CFLAGS="$CFLAGS"
    save_CXXFLAGS="$CXXFLAGS"
    CFLAGS="$CFLAGS $opt"
    CXXFLAGS="$CXXFLAGS $opt"
    AC_TRY_COMPILE([],[],[$1="$[]$1 $opt";
      m4_if([$4],[verbose],[AC_MSG_RESULT(yes)])
    ],[
      m4_if([$4],[verbose],[AC_MSG_RESULT(no)])
      :
    ])
    CFLAGS="$save_CFLAGS"
    CXXFLAGS="$save_CXXFLAGS"
  done
  AC_LANG_POP($3)
])

# MOO_AC_CC_OPT(var,options,verbose)
AC_DEFUN([MOO_AC_CC_OPT],[
  __MOO_AC_CHECK_COMPILER_OPTIONS([$1],[$2],[C],[$3])
])

# MOO_AC_CXX_OPT(var,options)
AC_DEFUN([MOO_AC_CXX_OPT],[
  __MOO_AC_CHECK_COMPILER_OPTIONS([$1],[$2],[C++],[$3])
])

# MOO_AC_GCC_OPT(var,options)
AC_DEFUN([MOO_AC_GCC_OPT],[
  AC_REQUIRE([MOO_COMPILER])
  if test $MOO_GCC; then
    MOO_AC_CC_OPT([$1],[$2],[$3])
  fi
])

# MOO_AC_GXX_OPT(var,options)
AC_DEFUN([MOO_AC_GXX_OPT],[
  AC_REQUIRE([MOO_COMPILER])
  if test $MOO_GCC; then
    MOO_AC_CXX_OPT([$1],[$2],[$3])
  fi
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
