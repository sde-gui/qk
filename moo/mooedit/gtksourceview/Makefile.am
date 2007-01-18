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
	upstream/gtksourcelanguage-parser-1.c	\
	upstream/gtksourcelanguage-parser-2.c	\
	upstream/gtksourcelanguagesmanager.c	\
	upstream/gtksourcestyle.c		\
	upstream/gtksourcestylemanager.c	\
	upstream/gtksourcestylescheme.c		\
	upstream/gtktextregion.c		\
	upstream/gtksourcecontextengine.h	\
	upstream/gtksourceengine.h		\
	upstream/gtksourceiter.h		\
	upstream/gtksourcelanguage.h		\
	upstream/gtksourcelanguage-private.h	\
	upstream/gtksourcelanguagesmanager.h	\
	upstream/gtksourcestyle.h		\
	upstream/gtksourcestylemanager.h	\
	upstream/gtksourcestylescheme.h		\
	upstream/gtktextregion.h

mangled_sources =				\
	gtksourcecontextengine-mangled.c	\
	gtksourceengine-mangled.c		\
	gtksourceiter-mangled.c			\
	gtksourcelanguage-mangled.c		\
	gtksourcelanguage-parser-1-mangled.c	\
	gtksourcelanguage-parser-2-mangled.c	\
	gtksourcelanguagesmanager-mangled.c	\
	gtksourcestyle-mangled.c		\
	gtksourcestylemanager-mangled.c		\
	gtksourcestylescheme-mangled.c		\
	gtktextregion-mangled.c			\
	gtksourcecontextengine-mangled.h	\
	gtksourceengine-mangled.h		\
	gtksourceiter-mangled.h			\
	gtksourcelanguage-mangled.h		\
	gtksourcelanguage-private-mangled.h	\
	gtksourcelanguagesmanager-mangled.h	\
	gtksourcestyle-mangled.h		\
	gtksourcestylemanager-mangled.h		\
	gtksourcestylescheme-mangled.h		\
	gtktextregion-mangled.h

BUILT_SOURCES = $(mangled_sources)
DISTCLEANFILES = $(mangled_sources)

%-mangled.c: upstream/%.c upstream/mangle.sh
	$(SHELL) $(srcdir)/upstream/mangle.sh $(srcdir)/upstream/$*.c > $@.tmp && mv $@.tmp $@
%-mangled.h: upstream/%.h upstream/mangle.sh
	$(SHELL) $(srcdir)/upstream/mangle.sh $(srcdir)/upstream/$*.h > $@.tmp && mv $@.tmp $@

upstream/%.o:
	$(MAKE) $*-mangled.o

libgtksourceview_la_SOURCES =		\
	$(mangled_sources)		\
	gtksourceview-api.h
