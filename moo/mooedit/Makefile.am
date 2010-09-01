noinst_LTLIBRARIES = libmooedit.la
EXTRA_DIST =
BUILT_SOURCES =
CLEANFILES =

AM_CPPFLAGS = $(MOO_CPPFLAGS) -I.. -I$(top_srcdir)/doc
AM_CFLAGS = $(MOO_CFLAGS)
AM_CXXFLAGS = $(MOO_CXXFLAGS)

libmooedit_la_SOURCES =		\
	moocompletionsimple.c	\
	moocompletionsimple.h	\
	mootextcompletion.c	\
	mootextcompletion.h	\
	mootextpopup.c		\
	mootextpopup.h		\
	moocmdview.c		\
	moocmdview.h		\
	mooedit-accels.h	\
	mooeditaction.c		\
	mooeditaction-factory.c	\
	mooeditaction-factory.h	\
	mooeditaction.h		\
	mooedit-bookmarks.c	\
	mooedit-bookmarks.h	\
	mooedit.c		\
	mooedit.h		\
	mooeditconfig.c		\
	mooeditconfig.h		\
	mooeditdialogs.c	\
	mooeditdialogs.h	\
	mooedit-fileops.c	\
	mooedit-fileops.h	\
	mooeditfiltersettings.c	\
	mooeditfiltersettings.h	\
	mooedit-impl.h		\
	mooeditor.c		\
	mooeditor.h		\
	mooeditor-private.h	\
	mooeditprefs.c		\
	mooeditprefs.h		\
	mooeditprefspage.c	\
	mooedit-private.h	\
	mooedit-tests.h		\
	mooeditwindow.c		\
	mooeditwindow.h		\
	moofold.c		\
	moofold.h		\
	mooindenter.c		\
	mooindenter.h		\
	moolang.c		\
	moolang.h		\
	moolangmgr.c		\
	moolangmgr.h		\
	moolangmgr-private.h	\
	moolang-private.h	\
	moolinebuffer.c		\
	moolinebuffer.h		\
	moolinemark.c		\
	moolinemark.h		\
	moolineview.c		\
	moolineview.h		\
	moooutputfilter.c	\
	moooutputfilter.h	\
	mooplugin.c		\
	mooplugin.h		\
	mooplugin-loader.c	\
	mooplugin-loader.h	\
	mooplugin-macro.h	\
	mootextbox.c		\
	mootextbox.h		\
	mootextbtree.c		\
	mootextbtree.h		\
	mootextbuffer.c		\
	mootextbuffer.h		\
	mootextfind.c		\
	mootextfind.h		\
	mootextiter.h		\
	mootext-private.h	\
	mootextsearch.c		\
	mootextsearch.h		\
	mootextsearch-private.h	\
	mootextstylescheme.c	\
	mootextstylescheme.h	\
	mootextview.c		\
	mootextview.h		\
	mootextview-input.c	\
	mootextview-private.h	\
	mooprintpreview.h	\
	mooprintpreview.c	\
	mootextprint.c		\
	mootextprint-private.h	\
	mootextprint.h		\
	mooedit-enums.c		\
	mooedit-enums.h

EXTRA_DIST +=					\
	glade/mooprintpreview.glade		\
	glade/moopluginprefs.glade		\
	glade/mooeditprefs-view.glade		\
	glade/mooeditprefs-file.glade		\
	glade/mooeditprefs-filters.glade	\
	glade/mooeditprefs-general.glade	\
	glade/mooeditprefs-langs.glade		\
	glade/mooeditprogress.glade		\
	glade/mooeditsavemult.glade		\
	glade/mooprint.glade			\
	glade/mootextfind.glade			\
	glade/mootextfind-prompt.glade		\
	glade/mootextgotoline.glade		\
	glade/mooquicksearch.glade		\
	glade/moostatusbar.glade

BUILT_SOURCES +=				\
	glade/mooprintpreview-gxml.h		\
	glade/moopluginprefs-gxml.h		\
	glade/mooeditprefs-view-gxml.h		\
	glade/mooeditprefs-file-gxml.h		\
	glade/mooeditprefs-filters-gxml.h	\
	glade/mooeditprefs-general-gxml.h	\
	glade/mooeditprefs-langs-gxml.h		\
	glade/mooeditprogress-gxml.h		\
	glade/mooeditsavemult-gxml.h		\
	glade/mooprint-gxml.h			\
	glade/mootextfind-gxml.h		\
	glade/mootextfind-prompt-gxml.h		\
	glade/mootextgotoline-gxml.h		\
	glade/mooquicksearch-gxml.h		\
	glade/moostatusbar-gxml.h

CLEANFILES +=					\
	glade/mooprintpreview-gxml.h		\
	glade/moopluginprefs-gxml.h		\
	glade/mooeditprefs-view-gxml.h		\
	glade/mooeditprefs-file-gxml.h		\
	glade/mooeditprefs-filters-gxml.h	\
	glade/mooeditprefs-general-gxml.h	\
	glade/mooeditprefs-langs-gxml.h		\
	glade/mooeditprogress-gxml.h		\
	glade/mooeditsavemult-gxml.h		\
	glade/mooprint-gxml.h			\
	glade/mootextfind-gxml.h		\
	glade/mootextfind-prompt-gxml.h		\
	glade/mootextgotoline-gxml.h		\
	glade/mooquicksearch-gxml.h		\
	glade/moostatusbar-gxml.h

EXTRA_DIST += medit.xml mooedit.xml
BUILT_SOURCES += medit-ui.h mooedit-ui.h
CLEANFILES += medit-ui.h mooedit-ui.h

include langs/Makefile.incl

include ../../tools/rules.mk

# -%- strip:true -%-
