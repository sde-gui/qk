moofileview_headers =	        \
	moofileview.h

moofileview_sources =		\
	$(moofileview_headers)  \
	moobookmarkmgr.c	\
	moobookmarkmgr.h	\
	moobookmarkmgr-glade.h	\
	moobookmarkview.c	\
	moobookmarkview.h	\
	moocreatefolder-glade.h	\
	moofileviewdrop-glade.h	\
	moofile.c		\
	moofile.h		\
	moofile-private.h	\
	moofileentry.c		\
	moofileentry.h		\
	moofileprops-glade.h	\
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
	moofileview-ui.h	\
	moofoldermodel.c	\
	moofoldermodel.h	\
	moofoldermodel-private.h\
	mooiconview.c		\
	mooiconview.h		\
	mootreeview.c		\
	mootreeview.h		\
	symlink.h

if MOO_INSTALL_HEADERS
moofileview_includedir = $(moo_includedir)/moofileview
moofileview_include_HEADERS = $(moofileview_headers)
endif

moofileview_pixmaps =		\
	symlink.png		\
	symlink-small.png

BUILT_SOURCES =			\
	moofileview-ui.h	\
	moofileprops-glade.h	\
	moocreatefolder-glade.h	\
	moofileviewdrop-glade.h	\
	symlink.h		\
	moobookmarkmgr-glade.h

symlink.h: $(moofileview_pixmaps)
	gdk-pixbuf-csource --static --build-list		\
		SYMLINK_ARROW $(srcdir)/symlink.png		\
		SYMLINK_ARROW_SMALL $(srcdir)/symlink-small.png	\
			> symlink.h

XML2H = $(srcdir)/../mooutils/xml2h.sh

moofileview-ui.h: $(srcdir)/moofileview-ui.xml $(XML2H)
	sh $(XML2H) MOO_FILE_VIEW_UI $(srcdir)/moofileview-ui.xml > moofileview-ui.h
moofileprops-glade.h: $(srcdir)/glade/moofileprops.glade $(XML2H)
	sh $(XML2H) MOO_FILE_PROPS_GLADE_UI $(srcdir)/glade/moofileprops.glade > moofileprops-glade.h
moocreatefolder-glade.h: $(srcdir)/glade/moocreatefolder.glade $(XML2H)
	sh $(XML2H) MOO_CREATE_FOLDER_GLADE_UI $(srcdir)/glade/moocreatefolder.glade > moocreatefolder-glade.h
moobookmarkmgr-glade.h: $(srcdir)/glade/bookmark_editor.glade $(XML2H)
	sh $(XML2H) MOO_BOOKMARK_MGR_GLADE_UI $(srcdir)/glade/bookmark_editor.glade > moobookmarkmgr-glade.h
moofileviewdrop-glade.h: $(srcdir)/glade/drop.glade $(XML2H)
	sh $(XML2H) MOO_FILE_VIEW_DROP_GLADE_UI $(srcdir)/glade/drop.glade > moofileviewdrop-glade.h


EXTRA_DIST =				\
	glade/drop.glade		\
	glade/moofileprops.glade	\
	glade/moocreatefolder.glade	\
	glade/bookmark_editor.glade	\
	moofileview-ui.xml		\
	$(moofileview_pixmaps)

noinst_LTLIBRARIES = libmoofileview.la
libmoofileview_la_SOURCES = $(moofileview_sources)

AM_CFLAGS =			\
	-I..			\
	-I$(top_builddir)	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)
