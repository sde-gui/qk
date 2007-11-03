AC_DEFUN_ONCE([MOO_AC_OBJC],[
  MOO_COMPILER

  if $MOO_ICC; then
    MOO_USE_OBJC=no
  else
    MOO_USE_OBJC=auto
  fi

  AC_ARG_WITH([objc], AC_HELP_STRING([--without-objc], [do not use Objective-C]), [
    MOO_USE_OBJC=$with_objc
  ])

  if test "x$MOO_USE_OBJC" != "xno"; then
    _MOO_OBJC_CHECK([
      MOO_USE_OBJC=yes
      MOO_OBJC_LIBS="$MOO_FFI_LIBS $MOO_OBJC_LIBS"
      AC_DEFINE(MOO_USE_OBJC, 1, [Use Objective-C.])
      AC_MSG_NOTICE([Objective-C flags: $MOO_OBJCFLAGS $MOO_OBJC_LIBS])

      AC_LANG_SAVE
      AC_LANG_OBJC
      AC_CHECK_SIZEOF(BOOL,,[#include <objc/objc.h>])
      AC_LANG_RESTORE
    ],[
      MOO_USE_OBJC=no
      MOO_OBJCFLAGS=
      MOO_OBJC_LIBS=
    ])
  fi

  AC_SUBST(MOO_OBJCFLAGS)
  AC_SUBST(MOO_OBJC_LIBS)

  AM_CONDITIONAL(MOO_USE_OBJC, test $MOO_USE_OBJC = yes)
])

#############################################################################
## Try to find Foundation
##
AC_DEFUN([_MOO_OBJC_CHECK_FOUNDATION],[
  AC_MSG_CHECKING([for Foundation])

  AC_LANG_SAVE
  AC_LANG_OBJC
  saved_LIBS="$LIBS"
  LIBS="-lobjc $LIBS"

  _moo_ac_have_foundation=no

  # First try to compile and link without any stuff, OBJCFLAGS and LIBS might be good enough
  AC_LINK_IFELSE([AC_LANG_PROGRAM([#import <Foundation/Foundation.h>],[NSAutoreleasePool *pool = @<:@@<:@NSAutoreleasePool alloc@:>@ init@:>@;])],[
    _moo_ac_have_foundation=yes
    MOO_OBJC_LIBS="-lobjc"
    AC_MSG_RESULT([yes])
  ],[
    LIBS="$LIBS -framework Foundation -lobjc"
    AC_LINK_IFELSE([AC_LANG_PROGRAM([#import <Foundation/Foundation.h>],[NSAutoreleasePool *pool = @<:@@<:@NSAutoreleasePool alloc@:>@ init@:>@;])],[
      _moo_ac_have_foundation=yes
      MOO_OBJC_LIBS="-framework Foundation -lobjc"
      AC_MSG_RESULT([yes])
    ],[
      _moo_ac_have_foundation=no
    ])
  ])

  LIBS="$saved_LIBS"
  AC_LANG_RESTORE

  if test "$_moo_ac_have_foundation" = "no"; then
    PKG_CHECK_MODULES(OBJCX,[libobjcx],[
      MOO_OBJC_LIBS="$OBJCX_LIBS"
      MOO_OBJCFLAGS="$OBJCX_CFLAGS"
      AC_MSG_RESULT([SideStep])
      _moo_ac_have_foundation=yes
    ],[
      AC_MSG_RESULT([no])
    ])
  fi

  if test "$_moo_ac_have_foundation" = "yes"; then
    MOO_OBJC_USE_FOUNDATION=yes
    MOO_OBJCFLAGS="$MOO_OBJCFLAGS -DMOO_OBJC_USE_FOUNDATION"
  fi
])

#############################################################################
## Check if basic ObjC runtime is available
##
AC_DEFUN([_MOO_OBJC_CHECK_RUNTIME],[
  AC_MSG_CHECKING(whether Objective-C compiler works)

  AC_LANG_SAVE
  AC_LANG([Objective C])
  save_LIBS="$LIBS"
  LIBS="-lobjc $LIBS"

  AC_LINK_IFELSE([AC_LANG_PROGRAM(
  [#import <objc/Object.h>

  @interface Foo : Object {
  @private
    int blah;
  }
  - foo;
  @end

  @implementation Foo : Object
  - foo
  {
    return self;
  }
  @end
  ],
  [Foo *obj = @<:@Foo new@:>@;])],
  [AC_MSG_RESULT(yes)
  MOO_OBJC_LIBS="-lobjc"
  $1],
  [AC_MSG_RESULT(no)
  $2])

  LIBS="$save_LIBS"
  AC_LANG_RESTORE
])

AC_DEFUN([_MOO_CHECK_FFI],[
  AC_MSG_CHECKING(FFI library)

  MOO_FFI_LIBS="-lffi"
  save_LIBS="$LIBS"
  LIBS="$LIBS $MOO_FFI_LIBS"

  AC_LINK_IFELSE([AC_LANG_PROGRAM(
  [#include <ffi.h>],
  [ffi_call (0, 0, 0, 0);])],
  [
    AC_MSG_RESULT(yes)
    $1
  ],[
    AC_MSG_RESULT(no)
    $2
  ])

  LIBS="$save_LIBS"
])

AC_DEFUN([_MOO_OBJC_CHECK],[
  MOO_OBJC_LIBS=
  MOO_OBJCFLAGS=

  _MOO_CHECK_FFI([
    _MOO_OBJC_CHECK_RUNTIME([
      MOO_OBJC_USE_FOUNDATION=no
      dnl _MOO_OBJC_CHECK_FOUNDATION
      $1
    ],[
      :
      $2
    ])
  ],[
    :
    $2
  ])
])
