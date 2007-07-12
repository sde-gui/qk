moofileview_headers =	        \
	moofileview.h

moofileview_sources =		\
	$(moofileview_headers)  \
	moobookmarkmgr.c	\
	moobookmarkmgr.h	\
	moobookmarkview.c	\
	moobookmarkview.h	\
	moofile.c		\
	moofile.h		\
	moofile-private.h	\
	moofileentry.c		\
	moofileentry.h		\
	moofileicons.c		\
	moofileicons.h		\
	moofilesystem.c		\
	moofilesystem.h		\
	moofileview.c		\
	moofileview-aux.h	\
	moofileview-dialogs.c	\
	moofileview-dialogs.h	\
	moofileview-impl.h	\
	moofileview-private.h	\
	moofileview-tools.c	\
	moofileview-tools.h	\
	moofolder-private.h	\
	moofolder.c		\
	moofolder.h		\
	moofoldermodel.c	\
	moofoldermodel.h	\
	moofoldermodel-private.h\
	mooiconview.c		\
	mooiconview.h		\
	mootreeview.c		\
	mootreeview.h

if MOO_INSTALL_HEADERS
moofileview_includedir = $(moo_includedir)/moofileview
moofileview_include_HEADERS = $(moofileview_headers)
endif

moofileview_pixmaps =		\
	symlink.png		\
	symlink-small.png

nodist_moofileview_sources =	\
	moofileview-ui.h	\
	symlink.h
BUILT_SOURCES =			\
	moofileview-ui.h	\
	symlink.h
CLEANFILES =			\
	moofileview-ui.h	\
	symlink.h

symlink.h: $(moofileview_pixmaps)
	gdk-pixbuf-csource --static --build-list	\
		SYMLINK_ARROW $(srcdir)/symlink.png	\
		SYMLINK_ARROW_SMALL $(srcdir)/symlink-small.png > $@.tmp && mv $@.tmp $@

XML2H = $(srcdir)/../mooutils/xml2h.sh
moofileview-ui.h: $(srcdir)/moofileview-ui.xml $(XML2H)
	$(SHELL) $(XML2H) MOO_FILE_VIEW_UI $(srcdir)/moofileview-ui.xml > $@.tmp && \
	mv $@.tmp $@

@MOO_GLADE_SUBDIR_RULE@
glade_files =				\
	glade/moofileview-drop.glade	\
	glade/moofileprops.glade	\
	glade/moocreatefolder.glade	\
	glade/moobookmark-editor.glade
glade_sources = $(patsubst glade/%.glade,%-glade.h,$(glade_files))
nodist_moofileview_sources += $(glade_sources)
BUILT_SOURCES += $(glade_sources)
CLEANFILES += $(glade_sources)
EXTRA_DIST = $(glade_files)

EXTRA_DIST +=				\
	moofileview-ui.xml		\
	$(moofileview_pixmaps)

noinst_LTLIBRARIES = libmoofileview.la
libmoofileview_la_SOURCES = $(moofileview_sources)
nodist_libmoofileview_la_SOURCES = $(nodist_moofileview_sources)

AM_CFLAGS =			\
	-I..			\
	-I$(top_builddir)	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)
