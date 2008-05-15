AC_DEFUN_ONCE([MOO_AC_TOOLS],[
  moo_srcdir=`cd $srcdir && pwd`
  moo_srcdir="$moo_srcdir/$1"
  AC_SUBST(moo_srcdir)

  MOO_XML2H="$moo_srcdir/mooutils/xml2h.sh"
  AC_SUBST(MOO_XML2H)
])
