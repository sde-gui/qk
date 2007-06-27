DIST_SUBDIRS = upstream

noinst_LTLIBRARIES = libgtksourceview.la

AM_CFLAGS =				\
	-I$(srcdir)/..			\
	-I..				\
	-I$(srcdir)/../..		\
	-I../..				\
	-I$(srcdir)/../../mooutils	\
	-I$(top_builddir)		\
	$(MOO_CFLAGS)			\
	$(MOO_DEBUG_CFLAGS)

upstream_sources =				\
	upstream/gtksourcecontextengine.c	\
	upstream/gtksourceengine.c		\
	upstream/gtksourceiter.c		\
	upstream/gtksourcelanguage.c		\
	upstream/gtksourcelanguage-noxml.c	\
	upstream/gtksourcelanguage-parser-1.c	\
	upstream/gtksourcelanguage-parser-2.c	\
	upstream/gtksourcelanguagemanager.c	\
	upstream/gtksourcestyle.c		\
	upstream/gtksourcestylemanager.c	\
	upstream/gtksourcestylescheme.c		\
	upstream/gtksourcestylescheme-noxml.c	\
	upstream/gtktextregion.c		\
	upstream/gtksourcecontextengine.h	\
	upstream/gtksourceengine.h		\
	upstream/gtksourceiter.h		\
	upstream/gtksourcelanguage.h		\
	upstream/gtksourcelanguage-private.h	\
	upstream/gtksourcelanguagemanager.h	\
	upstream/gtksourcestyle-private.h	\
	upstream/gtksourcestyle.h		\
	upstream/gtksourcestylemanager.h	\
	upstream/gtksourcestylescheme.h		\
	upstream/gtktextregion.h		\
	upstream/gtksourceview-utils.h		\
	upstream/gtksourceview-utils.c

gtksourceview_sources =				\
	gtksourcecontextengine-mangled.c	\
	gtksourceengine-mangled.c		\
	gtksourceiter-mangled.c			\
	gtksourcelanguagemanager-mangled.c	\
	gtksourcestyle-mangled.c		\
	gtksourcestylemanager-mangled.c		\
	gtktextregion-mangled.c			\
	gtksourceview-utils-mangled.c		\
	gtksourcecontextengine-mangled.h	\
	gtksourceengine-mangled.h		\
	gtksourceiter-mangled.h			\
	gtksourcelanguagemanager-mangled.h	\
	gtksourcestyle-private-mangled.h	\
	gtksourcestyle-mangled.h		\
	gtksourcestylemanager-mangled.h		\
	gtksourcestylescheme-mangled.h		\
	gtktextregion-mangled.h			\
	gtksourceview-utils-mangled.h

xml_sources =					\
	gtksourcestylescheme-mangled.c		\
	gtksourcelanguage-mangled.c		\
	gtksourcelanguage-parser-1-mangled.c	\
	gtksourcelanguage-parser-2-mangled.c	\
	gtksourcelanguage-mangled.h		\
	gtksourcelanguage-private-mangled.h

no_xml_sources =				\
	gtksourcelanguage-noxml-mangled.c	\
	gtksourcestylescheme-noxml-mangled.c

BUILT_SOURCES = $(xml_sources) $(no_xml_sources) $(gtksourceview_sources)
DISTCLEANFILES = $(xml_sources) $(no_xml_sources) $(gtksourceview_sources)

%-mangled.c: upstream/%.c upstream/mangle.sh
	$(SHELL) $(srcdir)/upstream/mangle.sh $(srcdir)/upstream/$*.c > $@.tmp && mv $@.tmp $@
%-mangled.h: upstream/%.h upstream/mangle.sh
	$(SHELL) $(srcdir)/upstream/mangle.sh $(srcdir)/upstream/$*.h > $@.tmp && mv $@.tmp $@

upstream/%.lo:
	$(MAKE) $*-mangled.lo

libgtksourceview_la_SOURCES =		\
	$(gtksourceview_sources)	\
	gtksourceview-i18n.h		\
	gtksourceview-marshal.h		\
	gtksourceview-api.h

if MOO_USE_XML
libgtksourceview_la_SOURCES += $(xml_sources)
EXTRA_DIST = $(no_xml_sources)
else
libgtksourceview_la_SOURCES += $(no_xml_sources)
EXTRA_DIST = $(xml_sources)
endif
