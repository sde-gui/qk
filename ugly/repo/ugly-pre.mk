# begin ugly-pre.mk
# -%- lang: makefile; indent-width: 8; use-tabs: true -%-

EXTRA_DIST =
BUILT_SOURCES =
CLEANFILES =
DISTCLEANFILES =

EXTRA_DIST +=					\
    Makefile.ug

BDIST_FILES =
BDIST_EXTRA =

UGLY = $(top_srcdir)/ugly/ugly
UGLY_DEPS =					\
	$(UGLY)					\
	$(top_srcdir)/ugly/repo/ugly-pre.mk	\
	$(top_srcdir)/ugly/repo/ugly-post.mk	\
	$(top_srcdir)/ugly/repo/bdist.mk

UGLY_ALL_TARGETS =
UGLY_CLEAN_TARGETS =

BUILT_SOURCES += ugly-pre-build-stamp
CLEANFILES += ugly-pre-build-stamp
UGLY_PRE_BUILD_TARGETS =
ugly-pre-build-stamp: $(UGLY_PRE_BUILD_TARGETS)
	echo stamp > ugly-pre-build-stamp

UGLY_SUBDIRS =
UGLY_PRE_BUILD_TARGETS += ugly-subdirs-stamp
CLEANFILES += ugly-pre-build-stamp
ugly-subdirs-stamp: $(UGLY_SUBDIRS) Makefile $(top_srcdir)/ugly/repo/ugly-subdir-Makefile
	@if test -n "$(UGLY_SUBDIRS)"; then \
	  for d in $(UGLY_SUBDIRS); do \
	    mkdir -p $$d || exit 1; \
	    cp $(top_srcdir)/ugly/repo/ugly-subdir-Makefile $$d/Makefile || exit 1; \
	  done; \
	fi
	echo stamp > ugly-subdirs-stamp

# end ugly-pre.mk
