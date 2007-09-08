DIST_SUBDIRS = pcre xdgmime newgtk

subdirs =
libs =
if MOO_BUILD_PCRE
subdirs += pcre
libs += pcre/libpcre.la
endif
if MOO_USE_XDGMIME
subdirs += xdgmime
libs += xdgmime/libxdgmime.la
endif
if !GLIB_2_14
subdirs += newgtk
libs += newgtk/libnewgtk.la
endif
SUBDIRS = $(subdirs) .

win32_sources =				\
	mooutils-win32.c

thread_sources =			\
	mooutils-thread.c		\
	mooutils-thread.h

mooutils_include_headers =		\
	mooaction.h			\
	mooactionbase.h			\
	mooactioncollection.h		\
	moobigpaned.h			\
	mooclosure.h			\
	moocombo.h			\
	mooentry.h			\
	moofiledialog.h			\
	moofiltermgr.h			\
	mooglade.h			\
	moohelp.h			\
	moohistorycombo.h		\
	moohistorylist.h		\
	mooi18n.h			\
	moomarkup.h			\
	moomenuaction.h			\
	moomenumgr.h			\
	moomenutoolbutton.h		\
	moonotebook.h			\
	moopane.h			\
	moopaned.h			\
	mooprefs.h			\
	mooprefsdialog.h		\
	mooprefsdialogpage.h		\
	moopython.h			\
	moostock.h			\
	moouixml.h			\
	mooundo.h			\
	mooutils-fs.h			\
	mooutils-debug.h		\
	mooutils-gobject.h		\
	mooutils-misc.h			\
	moowindow.h

mooutils_sources =			\
	$(mooutils_include_headers)	\
	mooaccel.h			\
	mooaccel.c			\
	mooaccelbutton.c		\
	mooaccelbutton.h		\
	mooaccelprefs.h			\
	mooaccelprefs.c			\
	mooaction.c			\
	mooaction-private.h		\
	mooactionbase.c			\
	mooactionbase-private.h		\
	mooactioncollection.c		\
	mooactionfactory.h		\
	mooactionfactory.c		\
	mooactiongroup.h		\
	mooactiongroup.c		\
	moobigpaned.c			\
	mooclosure.c			\
	moocombo.c			\
	moocompat.c			\
	moocompat.h			\
	moodialogs.c			\
	moodialogs.h			\
	mooencodings.c			\
	mooencodings.h			\
	mooencodings-data.h		\
	mooentry.c			\
	moofiledialog.c			\
	moofilewatch.c			\
	moofilewatch.h			\
	moofiltermgr.c			\
	moofontsel.c			\
	moofontsel.h			\
	mooglade.c			\
	moohelp.c			\
	moohistorycombo.c		\
	moohistorylist.c		\
	mooi18n.c			\
	moomarkup.c			\
	moomenuaction.c			\
	moomenumgr.c			\
	moomenutoolbutton.c		\
	moonotebook.c			\
	mooobjc.h			\
	moopane.c			\
	moopaned.c			\
	mooprefs.c			\
	mooprefsdialog.c		\
	mooprefsdialogpage.c		\
	moospawn.h			\
	moospawn.c			\
	moostock.c			\
	mootype-macros.h		\
	moouixml.c			\
	mooundo.c			\
	mooutils-fli.c			\
	mooutils-fs.c			\
	mooutils-gobject-private.h	\
	mooutils-gobject.c		\
	mooutils-misc.c			\
	mooutils-treeview.h		\
	mooutils-treeview.c		\
	moowindow.c			\
	stock-file-24.h			\
	stock-file-selector-24.h	\
	stock-select-all-16.h		\
	stock-select-all-24.h		\
	stock-terminal-24.h

mooobjc.lo: mooutils-fli.lo
moopython.lo: mooutils-fli.lo

objc_sources =				\
	moocobject.h			\
	moocobject.m

if MOO_USE_OBJC
mooutils_sources += $(objc_sources)
endif

if MOO_OS_MINGW
mooutils_sources += $(thread_sources) $(win32_sources)
else
if MOO_ENABLE_TESTS
mooutils_sources += $(thread_sources)
endif
endif

EXTRA_DIST =				\
	moomarshals.list		\
	py2h.sh				\
	xml2h.sh			\
	$(thread_sources)		\
	$(win32_sources)		\
	$(objc_sources)			\
	pixmaps/medit.png		\
	pixmaps/medit.ico		\
	pixmaps/hide.png		\
	pixmaps/close.png		\
	pixmaps/sticky.png		\
	pixmaps/detach.png		\
	pixmaps/attach.png		\
	pixmaps/keepontop.png

BUILT_SOURCES =				\
	moomarshals.c			\
	moomarshals.h
nodist_mooutils_sources =		\
	moomarshals.c			\
	moomarshals.h
CLEANFILES =				\
	moomarshals.c			\
	moomarshals.h

moomarshals.c: moomarshals.list
	echo '#include "moomarshals.h"' > $@.tmp &&	\
	glib-genmarshal --prefix=_moo_marshal --body	\
	       $(srcdir)/moomarshals.list >> $@.tmp &&	\
	mv $@.tmp $@
moomarshals.h: moomarshals.list
	glib-genmarshal --prefix=_moo_marshal --header	\
	       $(srcdir)/moomarshals.list > $@.tmp &&	\
	mv $@.tmp $@

@MOO_GLADE_SUBDIR_RULE@
glade_files =				\
	glade/mooaccelbutton.glade	\
	glade/mooaccelprefs.glade	\
	glade/moologwindow.glade
glade_sources = $(patsubst glade/%.glade,%-glade.h,$(glade_files))
nodist_mooutils_sources += $(glade_sources)
BUILT_SOURCES += $(glade_sources)
CLEANFILES += $(glade_sources)
EXTRA_DIST += $(glade_files)

mooutils_pixmaps =			\
	$(srcdir)/pixmaps/medit.png	\
	$(srcdir)/pixmaps/hide.png	\
	$(srcdir)/pixmaps/close.png	\
	$(srcdir)/pixmaps/detach.png	\
	$(srcdir)/pixmaps/attach.png	\
	$(srcdir)/pixmaps/keepontop.png	\
	$(srcdir)/pixmaps/sticky.png

CLEANFILES += stock-moo.h
BUILT_SOURCES += stock-moo.h
nodist_mooutils_sources += stock-moo.h
stock-moo.h: $(mooutils_pixmaps)
	gdk-pixbuf-csource --static --build-list			\
		MEDIT_ICON $(srcdir)/pixmaps/medit.png			\
		MOO_HIDE_ICON $(srcdir)/pixmaps/hide.png		\
		MOO_CLOSE_ICON $(srcdir)/pixmaps/close.png		\
		MOO_STICKY_ICON $(srcdir)/pixmaps/sticky.png		\
		MOO_DETACH_ICON $(srcdir)/pixmaps/detach.png		\
		MOO_ATTACH_ICON $(srcdir)/pixmaps/attach.png		\
		MOO_KEEP_ON_TOP_ICON $(srcdir)/pixmaps/keepontop.png	\
			> $@.tmp && mv $@.tmp $@

noinst_LTLIBRARIES = libmooutils.la
libmooutils_la_SOURCES = $(mooutils_sources)
nodist_libmooutils_la_SOURCES =	$(nodist_mooutils_sources)
libmooutils_la_LIBADD = $(libs)

AM_CFLAGS =					\
	-I$(top_builddir)			\
	-I..					\
	$(MOO_CFLAGS)				\
	$(MOO_DEBUG_CFLAGS)			\
	-DMOO_LOCALE_DIR=\"$(datadir)/locale\"	\
	-DMOO_HELP_DIR=\"$(htmldir)/help\"
AM_OBJCFLAGS = $(AM_CFLAGS) $(MOO_OBJCFLAGS)

if MOO_INSTALL_HEADERS
mooutils_includedir = $(moo_includedir)/mooutils
mooutils_include_HEADERS = $(mooutils_include_headers)
endif
