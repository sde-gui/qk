noinst_LTLIBRARIES = libmoofileview.la

AM_CPPFLAGS = $(MOO_CPPFLAGS) -I.. -I$(top_srcdir)/doc
AM_CFLAGS = $(MOO_CFLAGS)
AM_CXXFLAGS = $(MOO_CXXFLAGS)

BUILT_SOURCES =
EXTRA_DIST =
CLEANFILES =

libmoofileview_la_SOURCES =	\
	moobookmarkmgr.c	\
	moobookmarkmgr.h	\
	moobookmarkview.c	\
	moobookmarkview.h	\
	moofile.c		\
	moofile.h		\
	moofile-private.h	\
	moofileentry.c		\
	moofileentry.h		\
	moofilesystem.c		\
	moofilesystem.h		\
	moofileview.c		\
	moofileview.h		\
	moofileview-accels.h	\
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

EXTRA_DIST +=				\
	glade/moofileview-drop.glade	\
	glade/moofileprops.glade	\
	glade/moocreatefolder.glade	\
	glade/moobookmark-editor.glade

BUILT_SOURCES +=			\
	glade/moofileview-drop-gxml.h	\
	glade/moofileprops-gxml.h	\
	glade/moocreatefolder-gxml.h	\
	glade/moobookmark-editor-gxml.h

CLEANFILES +=				\
	glade/moofileview-drop-gxml.h	\
	glade/moofileprops-gxml.h	\
	glade/moocreatefolder-gxml.h	\
	glade/moobookmark-editor-gxml.h

EXTRA_DIST += moofileview.xml
BUILT_SOURCES += moofileview-ui.h
CLEANFILES += moofileview-ui.h

include ../../tools/rules.mk

# -%- strip:true -%-
