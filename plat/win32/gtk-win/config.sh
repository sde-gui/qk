export mgwx64=false

if $mgwx64; then
  export mgwplatform="x64"
else
  export mgwplatform="x86"
fi

if [ -z "$OSTYPE" -o "$OSTYPE" != "cygwin" ]; then
  export mgwsystem=linux
else
  export mgwsystem=cygwin
fi
if [ $mgwsystem = linux ]; then
  export mgwlinux=true
  export mgwcygwin=false
else
  export mgwlinux=false
  export mgwcygwin=true
fi

if [ $mgwplatform = 'x64' ]; then
  export libgccdll=/usr/lib/gcc/x86_64-w64-mingw32/*-win32/libgcc_s_sjlj-1.dll
  export libstdcppdll=/usr/lib/gcc/x86_64-w64-mingw32/*-win32/libstdc++*.dll
else
  export libgccdll=/usr/lib/gcc/i686-w64-mingw32/*-win32/libgcc_s_sjlj-1.dll
  export libstdcppdll=/usr/lib/gcc/i686-w64-mingw32/*-win32/libstdc++*.dll
fi

export moosrcdir=$HOME/projects/medit/moo
export mgwbuildroot=$HOME/projects/gtk-win-build
export mgwpythondotver=2.7

export mgwpythonver=`echo $mgwpythondotver | sed 's/[.]//'`

if $mgwlinux; then
  if [ $mgwplatform = 'x64' ]; then
    export mgwpythoninstdir=$HOME/.wine64/drive_c/Python$mgwpythonver
    export mgwpythonsystem32dir=$HOME/.wine64/drive_c/windows/system32
    export mgwprogramfilesdir="$HOME/.wine64/drive_c/Program Files (x86)"
  else
    export mgwpythoninstdir=$HOME/.wine/drive_c/Python$mgwpythonver
    export mgwpythonsystem32dir=$HOME/.wine/drive_c/windows/system32
    export mgwprogramfilesdir="$HOME/.wine/drive_c/Program Files"
  fi
else
  export mgwpythoninstdir=/cygdrive/c/Tools/Python$mgwpythonver
  export mgwpythonsystem32dir=/cygdrive/c/Windows/SysWOW64
fi

export mgwdir_s=$mgwdir
export mgwdir=`cd $mgwdir && pwd`

if [ -z "$mgwconfig" ]; then
  export mgwconfig=release
fi

case "$1" in
  --debug)
    mgwconfig=debug
    shift
    ;;
  --release)
    mgwconfig=release
    shift
    ;;
  --reldbg)
    mgwconfig=reldbg
    shift
    ;;
esac

export mgwplatsuffix=
if [ $mgwplatform = 'x64' ]; then
  export mgwplatsuffix="-x64"
fi

export mgwjhbuildsrcdir=$mgwbuildroot/jhbuild
export mgwjhbuilddir=$mgwbuildroot/jhbuild-bin

export mgwdestdir=$mgwbuildroot/bdist-$mgwconfig$mgwplatsuffix

export mgwbuilddir=$mgwbuildroot/$mgwconfig$mgwplatsuffix
export mgwbuilddir_s=$mgwbuildroot/$mgwconfig$mgwplatsuffix
export mgwsourcedir=$mgwbuilddir/source
export mgwtargetdir=$mgwbuilddir/target
export mgwsourcedir_s=$mgwbuilddir_s/source
export mgwtargetdir_s=$mgwbuilddir_s/target
export mgwdistdir=$mgwbuildroot/dist-$mgwconfig$mgwplatsuffix
