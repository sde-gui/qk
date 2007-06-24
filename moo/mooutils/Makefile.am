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
	mooutils-gobject.h		\
	mooutils-misc.h			\
	moowindow.h

mooutils_sources =			\
	$(mooutils_include_headers)	\
	mooaccel.h			\
	mooaccel.c			\
	mooaccelbutton-glade.h		\
	mooaccelbutton.c		\
	mooaccelbutton.h		\
	mooaccelprefs.h			\
	mooaccelprefs.c			\
	mooaccelprefs-glade.h		\
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
	moohistorycombo.c		\
	moohistorylist.c		\
	mooi18n.c			\
	moologwindow-glade.h		\
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
	moopython.c			\
	moospawn.h			\
	moospawn.c			\
	moostock.c			\
	mootype-macros.h		\
	moouixml.c			\
	mooundo.c			\
	mooutils-debug.h		\
	mooutils-fs.c			\
	mooutils-gobject-private.h	\
	mooutils-gobject.c		\
	mooutils-misc.c			\
	mooutils-treeview.h		\
	mooutils-treeview.c		\
	moowindow.c			\
	stock-file-24.h			\
	stock-file-selector-24.h	\
	stock-moo.h			\
	stock-select-all-16.h		\
	stock-select-all-24.h		\
	stock-terminal-24.h

if MOO_OS_MINGW
mooutils_sources += $(thread_sources) $(win32_sources)
else
if MOO_ENABLE_TESTS
mooutils_sources += $(thread_sources)
endif
endif

nodist_mooutils_sources =		\
	moomarshals.c			\
	moomarshals.h
DISTCLEANFILES =			\
	moomarshals.c			\
	moomarshals.h

EXTRA_DIST =				\
	moomarshals.list		\
	py2h.sh				\
	xml2h.sh			\
	$(thread_sources)		\
	$(win32_sources)		\
	glade/accelbutton.glade		\
	glade/accelprefs.glade		\
	glade/moologwindow.glade	\
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
	moomarshals.h			\
	mooaccelbutton-glade.h		\
	mooaccelprefs-glade.h		\
	moologwindow-glade.h		\
	stock-moo.h

moomarshals.c: moomarshals.list
	echo '#include "moomarshals.h"' > $@.tmp &&	\
	glib-genmarshal --prefix=_moo_marshal --body	\
	       $(srcdir)/moomarshals.list >> $@.tmp &&	\
	mv $@.tmp $@
moomarshals.h: moomarshals.list
	glib-genmarshal --prefix=_moo_marshal --header	\
	       $(srcdir)/moomarshals.list > $@.tmp &&	\
	mv $@.tmp $@

XML2H = $(srcdir)/xml2h.sh
mooaccelbutton-glade.h: $(srcdir)/glade/accelbutton.glade $(XML2H)
	$(SHELL) $(XML2H) MOO_ACCEL_BUTTON_GLADE_UI \
	   $(srcdir)/glade/accelbutton.glade > $@.tmp && \
	mv $@.tmp $@
mooaccelprefs-glade.h: $(srcdir)/glade/accelprefs.glade $(XML2H)
	$(SHELL) $(XML2H) MOO_ACCEL_PREFS_GLADE_UI \
	   $(srcdir)/glade/accelprefs.glade > $@.tmp && \
	mv $@.tmp $@
moologwindow-glade.h: $(srcdir)/glade/moologwindow.glade $(XML2H)
	$(SHELL) $(XML2H) MOO_LOG_WINDOW_GLADE_UI \
	   $(srcdir)/glade/moologwindow.glade > $@.tmp && \
	mv $@.tmp $@

mooutils_pixmaps =			\
	$(srcdir)/pixmaps/medit.png	\
	$(srcdir)/pixmaps/hide.png	\
	$(srcdir)/pixmaps/close.png	\
	$(srcdir)/pixmaps/detach.png	\
	$(srcdir)/pixmaps/attach.png	\
	$(srcdir)/pixmaps/keepontop.png	\
	$(srcdir)/pixmaps/sticky.png

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

AM_CFLAGS =			\
	-I$(top_builddir)	\
	-I..			\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)	\
	-DMOO_LOCALE_DIR=\"$(datadir)/locale\"

if MOO_INSTALL_HEADERS
mooutils_includedir = $(moo_includedir)/mooutils
mooutils_include_HEADERS = $(mooutils_include_headers)
endif
