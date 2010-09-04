noinst_LTLIBRARIES = libmomscript.la

AM_CPPFLAGS = $(MOO_CPPFLAGS) -I$(top_srcdir)/moo
AM_CFLAGS = $(MOO_CFLAGS)
AM_CXXFLAGS = $(MOO_CXXFLAGS)

libmomscript_la_SOURCES =		\
	momscript-types.h		\
	momscript-lua.h			\
	momscript-lua.cpp		\
	momscript-classes.h		\
	momscript-classes.cpp		\
	momscript-classes-meta.h	\
	momscript.h			\
	momscript.cpp			\
	momscript-api.h			\
	momscript-variant.h

BUILT_SOURCES = momscript-classes-meta.h
CLEANFILES = momscript-classes-meta.h
EXTRA_DIST = genmeta.py
momscript-classes-meta.h: momscript-classes.h genmeta.py
	$(PYTHON) $(srcdir)/genmeta.py $(srcdir)/momscript-classes.h > momscript-classes-meta.h.tmp && \
		mv momscript-classes-meta.h.tmp momscript-classes-meta.h

momscript.t2t: momscript-classes.h momscript-classes.cpp gendoc.py
	$(PYTHON) $(srcdir)/gendoc.py $(srcdir)/momscript-classes.h $(srcdir)/momscript-classes.cpp > $(srcdir)/momscript.t2t.tmp && \
		mv $(srcdir)/momscript.t2t.tmp $(srcdir)/momscript.t2t
