DIST_SUBDIRS = upstream

noinst_LTLIBRARIES = libgtksourceview.la

AM_CFLAGS =				\
	-I..				\
	-I$(srcdir)/..			\
	$(MOO_CFLAGS)			\
	$(MOO_DEBUG_CFLAGS)

upstream_sources =				\
	upstream/gtksourcecontextengine.c	\
	upstream/gtksourcecontextengine.h	\
	upstream/gtksourceengine.c		\
	upstream/gtksourceengine.h		\
	upstream/gtksourceiter.c		\
	upstream/gtksourceiter.h		\
	upstream/gtksourcelanguage-noxml.c	\
	upstream/gtksourcelanguage-parser-1.c	\
	upstream/gtksourcelanguage-parser-2.c	\
	upstream/gtksourcelanguage-private.h	\
	upstream/gtksourcelanguage.c		\
	upstream/gtksourcelanguage.h		\
	upstream/gtksourcelanguagemanager.c	\
	upstream/gtksourcelanguagemanager.h	\
	upstream/gtksourcestyle-private.h	\
	upstream/gtksourcestyle.c		\
	upstream/gtksourcestyle.h		\
	upstream/gtksourcestylescheme-noxml.c	\
	upstream/gtksourcestylescheme.c		\
	upstream/gtksourcestylescheme.h		\
	upstream/gtksourcestyleschememanager.c	\
	upstream/gtksourcestyleschememanager.h	\
	upstream/gtksourceview-utils.c		\
	upstream/gtksourceview-utils.h		\
	upstream/gtktextregion.c		\
	upstream/gtktextregion.h

gtksourceview_sources =				\
	gtksourcecontextengine-mangled.c	\
	gtksourcecontextengine-mangled.h	\
	gtksourceengine-mangled.c		\
	gtksourceengine-mangled.h		\
	gtksourceiter-mangled.c			\
	gtksourceiter-mangled.h			\
	gtksourcelanguagemanager-mangled.c	\
	gtksourcelanguagemanager-mangled.h	\
	gtksourcestyle-mangled.c		\
	gtksourcestyle-mangled.h		\
	gtksourcestyle-private-mangled.h	\
	gtksourcestylescheme-mangled.h		\
	gtksourcestyleschememanager-mangled.c	\
	gtksourcestyleschememanager-mangled.h	\
	gtksourceview-utils-mangled.c		\
	gtksourceview-utils-mangled.h		\
	gtktextregion-mangled.c			\
	gtktextregion-mangled.h

xml_sources =					\
	gtksourcelanguage-mangled.c		\
	gtksourcelanguage-mangled.h		\
	gtksourcelanguage-parser-1-mangled.c	\
	gtksourcelanguage-parser-2-mangled.c	\
	gtksourcelanguage-private-mangled.h	\
	gtksourcestylescheme-mangled.c

no_xml_sources =				\
	gtksourcelanguage-noxml-mangled.c	\
	gtksourcestylescheme-noxml-mangled.c

BUILT_SOURCES = $(xml_sources) $(no_xml_sources) $(gtksourceview_sources)
CLEANFILES = $(xml_sources) $(no_xml_sources) $(gtksourceview_sources)

%-mangled.c: upstream/%.c upstream/mangle.sh
	$(SHELL) $(srcdir)/upstream/mangle.sh $(srcdir)/upstream/$*.c > $@.tmp && mv $@.tmp $@
%-mangled.h: upstream/%.h upstream/mangle.sh
	$(SHELL) $(srcdir)/upstream/mangle.sh $(srcdir)/upstream/$*.h > $@.tmp && mv $@.tmp $@

upstream/%.lo:
	$(MAKE) $*-mangled.lo

libgtksourceview_la_SOURCES =		\
	gtksourceview-i18n.h		\
	gtksourceview-marshal.h		\
	gtksourceview-api.h
nodist_libgtksourceview_la_SOURCES =	\
	$(gtksourceview_sources)

if MOO_USE_XML
nodist_libgtksourceview_la_SOURCES += $(xml_sources)
EXTRA_DIST = $(no_xml_sources)
else
nodist_libgtksourceview_la_SOURCES += $(no_xml_sources)
EXTRA_DIST = $(xml_sources)
endif
