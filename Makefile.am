ACLOCAL_AMFLAGS = -I m4 $(ACLOCAL_FLAGS)

SUBDIRS = po po-gsv api doc moo

EXTRA_DIST =				\
	tools/genenums.py		\
	tools/glade2c.py		\
	tools/xml2h.py

if MOO_DEV_MODE
DISTCHECK_CONFIGURE_FLAGS = MAKEFLAGS=-j3 CFLAGS="-Werror -Wall -O2" CXXFLAGS="-Werror -Wall -O2"
endif

CLEANFILES =

include plat/win32/Makefile.incl

if MOO_OS_WIN32
CLEANFILES += plat/win32/installer.iss
plat/win32/installer.iss: $(top_srcdir)/plat/win32/installer.iss.in $(top_builddir)/config.status
	$(AM_V_at)$(MKDIR_P) plat/win32
	$(AM_V_GEN)cd $(top_builddir) && ./config.status --silent --file=plat/win32/installer.iss
installer: plat/win32/installer.iss
	$(MAKE) $(AM_MAKEFLAGS) install
	$(MEDIT_INNO_COMPILER) plat/win32/installer.iss
endif

test:
	$(MAKE) $(AM_MAKEFLAGS) all
	$(MAKE) $(AM_MAKEFLAGS) check
	$(MAKE) $(AM_MAKEFLAGS) uninstall
	$(MAKE) $(AM_MAKEFLAGS) check
	$(MAKE) $(AM_MAKEFLAGS) install
	$(MAKE) $(AM_MAKEFLAGS) installcheck
fullcheck:
	$(MAKE) $(AM_MAKEFLAGS) test
	$(MAKE) $(AM_MAKEFLAGS) distcheck

@MOO_PO_SUBDIRS_RULE@
