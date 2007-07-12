AC_DEFUN_ONCE([MOO_AC_OBJC],[
  MOO_USE_OBJC=auto
  AC_ARG_WITH([objc], AC_HELP_STRING([--without-objc], [do not use Objective-C]), [
    MOO_USE_OBJC=$with_objc
  ])

  if test "x$MOO_USE_OBJC" != "xno"; then
    _MOO_OBJC_CHECK([
      MOO_USE_OBJC=yes
      MOO_OBJC_LIBS="-lobjc"
      AC_DEFINE(MOO_USE_OBJC, 1, [Use Objective-C.])
    ],[
      MOO_WARN_OBJC="Objective-C support is disabled, editor user tools will be disabled"
      MOO_USE_OBJC=no
      MOO_OBJC_LIBS=""
    ])
  fi

  AC_SUBST(MOO_OBJC_LIBS)
  AM_CONDITIONAL(MOO_USE_OBJC, test $MOO_USE_OBJC = yes)
])

AC_DEFUN([_MOO_OBJC_CHECK],[
  AC_MSG_CHECKING(whether Objective-C compiler works)

  AC_LANG_SAVE
  AC_LANG([Objective C])
  save_LIBS=$LIBS
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
$1],
[AC_MSG_RESULT(no)
$2])

  LIBS="$save_LIBS"
  AC_LANG_RESTORE
])
