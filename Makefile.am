AUTOMAKE_OPTIONS = dist-bzip2 no-dist-gzip foreign nostdinc

EXTRA_DIST =			\
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

if MOO_ENABLE_NLS
po_subdirs = po po-gsv
endif

if MOO_BUILD_MEDIT
SUBDIRS = . $(po_subdirs) doc moo medit
else
SUBDIRS = . $(po_subdirs) doc moo
endif

if MOO_ENABLE_TESTS
SUBDIRS += tests
else
if MOO_ENABLE_UNIT_TESTS
SUBDIRS += tests
endif
endif

ACLOCAL_AMFLAGS = -I m4

DISTCLEANFILES =		\
	configargs.h		\
	intltool-extract	\
	intltool-merge		\
	intltool-update		\
	po-subdirs-stamp-2	\
	po-subdirs-stamp

# DISTCHECK_CONFIGURE_FLAGS = --enable-unit-tests
distuninstallcheck_listfiles = find . -type f -print | grep -v share/mime/ | grep -v share/icons/hicolor

snapshot:
	$(MAKE) dist && mv $(PACKAGE)-$(VERSION).tar.bz2 $(PACKAGE)-`date +%F-%H-%M`.tar.bz2

win-test-stuff:
	$(MAKE) all && mkdir -p test-stuff/test-data && \
	cp moo/.libs/libmoo.dll test-stuff/ && \
	cp -r $(srcdir)/tests/data/* test-stuff/test-data/ && \
	cp tests/.libs/run-tests.exe test-stuff/
