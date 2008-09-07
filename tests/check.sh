#! /bin/sh

moo_repo=/home/muntyan/projects/moo

tmpdir=

go_to_tmp() {
  if [ -z "$tmpdir" ]; then
    tmpdir="`mktemp -t medit-test-tmp.XXXXXXXX`"
    echo "rm -f $tmpdir"
    rm -f "$tmpdir" || exit 1
    echo "mkdir $tmpdir"
    mkdir "$tmpdir" || exit 1
    echo "cd $tmpdir"
    cd "$tmpdir"
  fi
}

check_hg() {
  go_to_tmp

  echo hg clone $moo_repo moo
  hg clone $moo_repo moo || exit 1
  echo cd moo
  cd moo || exit 1

  ./autogen.sh || { echo "autogen.sh failed in $tmpdir"; exit 1; }
  ./configure || { echo "configure failed in $tmpdir"; exit 1; }
  make || { echo "make failed in $tmpdir"; exit 1; }
  make distcheck || { echo "make distcheck failed in $tmpdir"; exit 1; }

  cd ../..
  rm -r $tmpdir
  tmpdir=
}

usage() {
  echo "Usage: $0 hg"
}

if [ -z "$1" ]; then
  usage
  exit 1
fi

for arg; do
  case "$arg" in
    hg)
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
