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

# end ugly-pre.mk
