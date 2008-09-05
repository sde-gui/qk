#! /bin/sh

tipurl=http://mooedit.sourceforge.net/hg/hg.cgi/moo/archive/tip.tar.bz2

tmpdir=

go_to_tmp() {
  if [ -z "$tmpdir" ]; then
    tmpdir="`mktemp`"
    rm -f "$tmpdir" || exit 1
    echo "mkdir $tmpdir"
    mkdir "$tmpdir" || exit 1
    cd "$tmpdir"
  fi
}

check_hg() {
  go_to_tmp
  wget $tipurl || exit 1
  tar xjf tip.tar.bz2 || exit 1
  cd moo-* || exit 1

  ./autogen.sh || { echo "autogen.sh failed in $tmpdir"; exit 1; }
  ./configure || { echo "configure failed in $tmpdir"; exit 1; }
  make || { echo "make failed in $tmpdir"; exit 1; }
  make distcheck || { echo "make distcheck failed in $tmpdir"; exit 1; }

  cd ..
  rm -r $tmpdir
  tmpdir=
}

usage() {
  echo "Usage: $0 check_hg"
}

for arg; do
  case "$arg" in
    check_hg)
      check_hg || exit 1
      ;;
    --help)
      usage
      exit 0
      ;;
    *)
      usage
      exit 1
      ;;
  esac
done
