DIST_SUBDIRS = pcre xdgmime

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
SUBDIRS = $(subdirs) .

win32_sources =				\
	mooutils-win32.c

gmappedfile_sources =			\
	newgtk/gmappedfile.c		\
	newgtk/gmappedfile.h

gtk_2_4_sources =			\
	newgtk/gtkcolorbutton.c		\
	newgtk/gtkcolorbutton.h		\
	newgtk/gtkfontbutton.c		\
	newgtk/gtkfontbutton.h

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
	moopaned.h			\
	mooprefs.h			\
	mooprefsdialog.h		\
	mooprefsdialogpage.h		\
	moopython.h			\
	mooregex.h			\
	moostock.h			\
	moouixml.h			\
	mooundo.h			\
	mooutils-fs.h			\
	mooutils-gobject.h		\
	mooutils-misc.h			\
	moowindow.h

mooutils_sources =			\
	$(mooutils_include_headers)	\
	eggregex.h			\
	eggregex.c			\
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
	moopaned.c			\
	mooprefs.c			\
	mooprefsdialog.c		\
	mooprefsdialogpage.c		\
	moopython.c			\
	mooregex.c			\
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
	stock-about-16.h		\
	stock-about-24.h		\
	stock-edit-16.h			\
	stock-edit-24.h			\
	stock-moo.h			\
	stock-select-all-16.h		\
	stock-select-all-24.h		\
	stock-terminal-24.h

if !GLIB_2_10
mooutils_sources += $(gmappedfile_sources)
endif
if !GTK_2_4
mooutils_sources += $(moonewgtk_gtk_2_4_sources)
endif
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
	$(gmappedfile_sources)		\
	$(gtk_2_4_sources)		\
	$(thread_sources)		\
	$(win32_sources)		\
	glade/accelbutton.glade		\
	glade/accelprefs.glade		\
	glade/moologwindow.glade	\
	pixmaps/medit.png               \
	pixmaps/medit.ico               \
	pixmaps/close.png               \
	pixmaps/sticky.png              \
	pixmaps/detach.png              \
	pixmaps/attach.png              \
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
	$(srcdir)/pixmaps/close.png	\
	$(srcdir)/pixmaps/detach.png	\
	$(srcdir)/pixmaps/attach.png	\
	$(srcdir)/pixmaps/keepontop.png	\
	$(srcdir)/pixmaps/sticky.png

stock-moo.h: $(mooutils_pixmaps)
	gdk-pixbuf-csource --static --build-list			\
		MEDIT_ICON $(srcdir)/pixmaps/medit.png			\
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

if MOO_BUILD_PCRE
pcre_include = -I$(srcdir)/pcre
endif

AM_CFLAGS =			\
	-I$(top_builddir)	\
	-I..			\
	$(pcre_include)		\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)	\
	-DMOO_LOCALE_DIR=\"$(datadir)/locale\"

if MOO_INSTALL_HEADERS
mooutils_includedir = $(moo_includedir)/mooutils
mooutils_include_HEADERS = $(mooutils_include_headers)
endif
