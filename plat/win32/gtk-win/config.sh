export mgwdestdir=/tmp/gtk-win

export mgwdir_s=$mgwdir
export mgwdir=`cd $mgwdir && pwd`
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

export mgwdestgtkdir=$mgwdestdir/gtk-bin-$mgwconfig

export mgwbuilddir=$mgwdir/$mgwconfig
export mgwbuilddir_s=$mgwdir_s/$mgwconfig
export mgwsourcedir=$mgwbuilddir/source
export mgwtargetdir=$mgwbuilddir/target
export mgwsourcedir_s=$mgwbuilddir_s/source
export mgwtargetdir_s=$mgwbuilddir_s/target
