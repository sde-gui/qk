AUTOMAKE_OPTIONS = dist-bzip2 no-dist-gzip foreign nostdinc

m4files =			\
	m4/moo-comps.m4		\
	m4/moo-debug.m4		\
	m4/moo-dirs.m4		\
	m4/moo-docs.m4		\
	m4/moo-fam.m4		\
	m4/moo-flags.m4		\
	m4/moo-funcs.m4		\
	m4/moo-gtk.m4		\
	m4/moo-i18n.m4		\
	m4/moo-lib.m4		\
	m4/moo-lua.m4		\
	m4/moo-objc.m4		\
	m4/moo-os.m4		\
	m4/moo-pcre.m4		\
	m4/moo-progs.m4		\
	m4/moo-pygtk.m4		\
	m4/moo-python.m4	\
	m4/moo-version.m4	\
	m4/moo-xdgmime.m4	\
	m4/moo-xml.m4		\
	m4/moo-python-cross-mingw.m4

EXTRA_DIST =			\
	$(m4files)		\
	autogen.sh		\
	mingw-configure		\
	cygwin-configure	\
	makewin			\
	COPYING.GPL		\
	COPYING			\
	LICENSE			\
	THANKS			\
	intltool-extract.in	\
	intltool-merge.in	\
	intltool-update.in

@MOO_PO_SUBDIRS_RULE@
@MOO_PO_SUBDIRS_RULE2@
BUILT_SOURCES = po-subdirs-stamp po-subdirs-stamp-2

DIST_SUBDIRS = moo medit tests po po-gsv doc

if MOO_BUILD_MEDIT
SUBDIRS = . moo medit
else
SUBDIRS = . moo
endif

if MOO_ENABLE_TESTS
SUBDIRS += tests
endif

if MOO_ENABLE_NLS
SUBDIRS += po po-gsv
endif

if MOO_ENABLE_HELP
if MOO_BUILD_DOCS
SUBDIRS += doc
endif
endif

ACLOCAL_AMFLAGS = -I m4

DISTCLEANFILES =		\
	m4-stamp		\
	configargs.h		\
	intltool-extract	\
	intltool-merge		\
	intltool-update		\
	po-subdirs-stamp-2	\
	po-subdirs-stamp

DISTCHECK_CONFIGURE_FLAGS="--enable-docs"
distuninstallcheck_listfiles = find . -type f -print | grep -v share/mime/ | grep -v share/icons/hicolor

snapshot:
	$(MAKE) dist && mv $(PACKAGE)-$(VERSION).tar.bz2 $(PACKAGE)-`date +%F-%H-%M`.tar.bz2
