WARNING: this build environment builds binaries for medit, and it may or may
not be suitable for other purposes.

Windows binaries of medit are built on a Linux system using Mingw cross-compiler
and jhbuild.

Build requires Windows Python-2.7 to be present on the build machine, by default
it looks for it where wine would install it, see config.sh. Python is required to
build pycairo, pygobject, and pygtk, and to build medit itself (which uses pygtk).

To build:
    1) check and adjust config.sh
        mgwbuildroot: location of the jhbuild environment root - source and build
        directories and resulting binaries will be there.
        mgwpython*: python version and where its installation can be found. Default
        values are where wine installs Python-2.7.msi these days.
    2) Run
        ./make.sh [--debug] build
        ./make.sh [--debug] bdist

To build individual jhbuild modules, use
    ./mjhbuild [--debug] buildone <module>
mjhbuild is a thin wrapper around jhbuild which reads config.sh and starts jhbuild,
it passes all command line arguments (except --debug) to jhbuild.

gtk-win (where this file is) directory contents:
    mjhbuild.sh: wrapper around jhbuild
    extra: extra files which are installed with medit but are not built.
    jhbuildrc: directory which contains jhbuild configuration
        jhbuildrc: actual jhbuildrc file
        gtk.moduleset: jhbuild moduleset file

Build directory layout:
    - gtk-win-build: root directory for the build, set in config.sh.
        - release: root directory for the release build.
            - build: jhbuild build directory.
            - source: jhbuild source directory.
            - target: jhbuild target directory - $prefix parameter for configure,
                      where built software gets installed.
        - debug: same as release, but when mjhbuild is invoked with --debug option.
            ...
        - tarballs: jhbuild tarball directory.
        - bdist-release: directory which contains built binaries as they are distributed
                         with medit. This is the contents of release/target directory with
                         non-distributed files (e.g. headers) removed, plus extra files
                         copied by "make.sh bdist" command.
        - bdist-debug: same as bdist-release, but for debug configuration.
