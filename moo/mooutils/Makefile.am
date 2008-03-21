DIST_SUBDIRS = pcre xdgmime newgtk

subdirs =
libs =
if MOO_BUILD_PCRE
subdirs += pcre
libs += pcre/libpcre.la
endif
subdirs += xdgmime
libs += xdgmime/libxdgmime.la
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
	mdhistorymgr.h			\
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
	mooutils-file.h			\
	mooutils-gobject.h		\
	mooutils-misc.h			\
	moowindow.h

mooutils_sources =			\
	$(mooutils_include_headers)	\
	mdhistorymgr.c			\
	mooappinput.h			\
	mooappinput.c			\
	mooapp-ipc.h			\
	mooapp-ipc.c			\
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
	moo-environ.h			\
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
	mooutils-mem.h			\
	mooutils-misc.c			\
	mooutils-treeview.h		\
	mooutils-treeview.c		\
	moowindow.c			\
	stock-medit.h			\
	stock-file-24.h			\
	stock-file-selector-24.h	\
	stock-select-all-16.h		\
	stock-select-all-24.h		\
	stock-terminal-24.h

moopython.lo: mooutils-fli.lo

if MOO_OS_MINGW
mooutils_sources += $(thread_sources) $(win32_sources)
else
if MOO_ENABLE_TESTS
mooutils_sources += $(thread_sources)
endif
endif

EXTRA_DIST =				\
	moo-intltool-merge		\
	py2h.sh				\
	xml2h.sh			\
	$(thread_sources)		\
	$(win32_sources)		\
	moowin32/mingw/fnmatch.h	\
	moowin32/mingw/netinet/in.h	\
	moowin32/mingw/sys/mman.h	\
	pixmaps/medit.png		\
	pixmaps/medit.ico		\
	pixmaps/hide.png		\
	pixmaps/close.png		\
	pixmaps/sticky.png		\
	pixmaps/detach.png		\
	pixmaps/attach.png		\
	pixmaps/keepontop.png

nodist_mooutils_sources =
BUILT_SOURCES =
CLEANFILES =

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
	$(srcdir)/pixmaps/hide.png	\
	$(srcdir)/pixmaps/close.png	\
	$(srcdir)/pixmaps/detach.png	\
	$(srcdir)/pixmaps/attach.png	\
	$(srcdir)/pixmaps/keepontop.png	\
	$(srcdir)/pixmaps/sticky.png

CLEANFILES += stock-moo.h stock-medit.h
BUILT_SOURCES += stock-moo.h stock-medit.h
nodist_mooutils_sources += stock-moo.h stock-medit.h
stock-moo.h: $(mooutils_pixmaps)
	gdk-pixbuf-csource --static --build-list			\
		MOO_HIDE_ICON $(srcdir)/pixmaps/hide.png		\
		MOO_CLOSE_ICON $(srcdir)/pixmaps/close.png		\
		MOO_STICKY_ICON $(srcdir)/pixmaps/sticky.png		\
		MOO_DETACH_ICON $(srcdir)/pixmaps/detach.png		\
		MOO_ATTACH_ICON $(srcdir)/pixmaps/attach.png		\
		MOO_KEEP_ON_TOP_ICON $(srcdir)/pixmaps/keepontop.png	\
			> $@.tmp && mv $@.tmp $@
stock-medit.h: pixmaps/medit.png
	gdk-pixbuf-csource --static --build-list			\
		MEDIT_ICON $(srcdir)/pixmaps/medit.png			\
			> $@.tmp && mv $@.tmp $@

BUILT_SOURCES += marshals.h
nodist_mooutils_sources += marshals.h
CLEANFILES += marshals.h
EXTRA_DIST += marshals.list
marshals.h: marshals.list
	glib-genmarshal --prefix=_moo_marshal --header	\
	       $(srcdir)/marshals.list > $@.tmp &&	\
	mv $@.tmp $@

noinst_LTLIBRARIES = libmooutils.la
libmooutils_la_SOURCES = $(mooutils_sources)
nodist_libmooutils_la_SOURCES =	$(nodist_mooutils_sources)
libmooutils_la_LIBADD = $(libs)

AM_CFLAGS =					\
	-I$(top_builddir)			\
	-I..					\
	-I$(top_srcdir)/doc			\
	-I$(top_builddir)/doc			\
	$(MOO_CFLAGS)				\
	$(MOO_DEBUG_CFLAGS)			\
	-DMOO_LOCALE_DIR=\"$(datadir)/locale\"	\
	-DMOO_HELP_DIR=\"$(htmldir)/help\"

if MOO_INSTALL_HEADERS
mooutils_includedir = $(moo_includedir)/mooutils
mooutils_include_HEADERS = $(mooutils_include_headers)
endif
