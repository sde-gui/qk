export mgwdir_s=$mgwdir
export mgwdir=`cd $mgwdir && pwd`
export mgwbuildroot=$HOME/projects/gtk-win-build
export mgwconfig=release

case "$1" in
  --debug)
    mgwconfig=debug
    shift
    ;;
  --release)
    mgwconfig=release
    shift
    ;;
esac

export mgwdestdir=$mgwbuildroot/bdist-$mgwconfig

export mgwbuilddir=$mgwbuildroot/$mgwconfig
export mgwbuilddir_s=$mgwbuildroot/$mgwconfig
export mgwsourcedir=$mgwbuilddir/source
export mgwtargetdir=$mgwbuilddir/target
export mgwsourcedir_s=$mgwbuilddir_s/source
export mgwtargetdir_s=$mgwbuilddir_s/target

export mgwpythoninstdir=$HOME/.wine/drive_c/Python27
export mgwpythonsystem32dir=$HOME/.wine/drive_c/windows/system32
