AUTOMAKE_OPTIONS = dist-bzip2 no-dist-gzip foreign nostdinc

m4files =			\
	m4/moo-comps.m4		\
	m4/moo-debug.m4		\
	m4/moo-dirs.m4		\
	m4/moo-fam.m4		\
	m4/moo-flags.m4		\
	m4/moo-funcs.m4		\
	m4/moo-gtk.m4		\
	m4/moo-i18n.m4		\
	m4/moo-lib.m4		\
	m4/moo-os.m4		\
	m4/moo-pcre.m4		\
	m4/moo-progs.m4		\
	m4/moo-pygtk.m4		\
	m4/moo-python.m4	\
	m4/moo-version.m4	\
	m4/moo-xdgmime.m4	\
	m4/moo-xml.m4

EXTRA_DIST =			\
	$(m4files)		\
	autogen.sh		\
	mingw-configure		\
	cygwin-configure	\
	makewin			\
	COPYING.lib		\
	THANKS			\
	intltool-extract.in	\
	intltool-merge.in	\
	intltool-update.in	\
	mkpotsubdirs

all-am: pot-subdirs-stamp m4-stamp

m4-stamp: $(m4files)
	echo stamp > m4-stamp
	for f in $(m4files); do						\
		if test $(srcdir)/$$f -nt $(srcdir)/configure; then	\
			touch $(srcdir)/configure.ac;			\
		fi							\
	done

pot-subdirs-stamp: po/POTFILES.in
	$(SHELL) $(srcdir)/mkpotsubdirs "$(srcdir)"
CLEANFILES = pot-subdirs-stamp

DIST_SUBDIRS = moo medit tests po
# need to create pot-subdirs-stamp before it goes to po/
if MOO_BUILD_MEDIT
SUBDIRS = . moo medit po
else
SUBDIRS = . moo po
endif

if MOO_ENABLE_TESTS
SUBDIRS += tests
endif

ACLOCAL_AMFLAGS = -I m4

DISTCLEANFILES =		\
	m4-stamp		\
	configargs.h		\
	intltool-extract	\
	intltool-merge		\
	intltool-update

distuninstallcheck_listfiles = find . -type f -print | grep -v share/mime/ | grep -v share/icons/hicolor
