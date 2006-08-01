DIST_SUBDIRS = pcre xdgmime

subdirs =
libs =
if MOO_BUILD_PCRE
subdirs += pcre
libs += pcre/libpcre.la
endif
if MOO_OS_UNIX
subdirs += xdgmime
libs += xdgmime/libxdgmime.la
endif
SUBDIRS = $(subdirs) .

gmappedfile_sources =		\
	newgtk/gmappedfile.c	\
	newgtk/gmappedfile.h

gtk_2_4_sources =		\
	newgtk/gtkcolorbutton.c	\
	newgtk/gtkcolorbutton.h	\
	newgtk/gtkfontbutton.c	\
	newgtk/gtkfontbutton.h

mooutils_include_headers =	\
	mooaccelbutton.h	\
	mooaction.h		\
	moobigpaned.h		\
	mooclosure.h		\
	moocmd.h		\
	moocombo.h		\
	moocommand.h		\
	mooconfig.h		\
	mooentry.h		\
	moofiltermgr.h		\
	mooglade.h		\
	moohistoryentry.h	\
	moohistorylist.h	\
	moolinklabel.h		\
	moomarkup.h		\
	moomenuaction.h		\
	moomenumgr.h		\
	moomenutoolbutton.h	\
	moonotebook.h		\
	moopaned.h		\
	mooprefs.h		\
	mooprefsdialog.h	\
	mooprefsdialogpage.h	\
	moopython.h		\
	moostock.h		\
	moouixml.h		\
	mooundo.h		\
	mooutils-fs.h		\
	mooutils-gobject.h	\
	mooutils-misc.h		\
	moowindow.h

mooutils_sources =			\
	$(mooutils_include_headers)	\
	eggregex.h			\
	eggregex.c			\
	mooaccel.h			\
	mooaccel.c			\
	mooaccelbutton-glade.h		\
	mooaccelbutton.c		\
	mooaccelprefs-glade.h		\
	mooaction.c			\
	mooactionfactory.h		\
	mooactionfactory.c		\
	moobigpaned.c			\
	mooclosure.c			\
	moocmd.c			\
	moocombo.c			\
	moocompat.c			\
	moocompat.h			\
	moocommand.c			\
	mooconfig.c			\
	moodialogs.c			\
	moodialogs.h			\
	mooentry.c			\
	moofiledialog.c			\
	moofiledialog.h			\
	moofilewatch.c			\
	moofilewatch.h			\
	moofiltermgr.c			\
	moofontsel.c			\
	moofontsel.h			\
	mooglade.c			\
	moohistoryentry.c		\
	moohistorylist.c		\
	moolinklabel.c			\
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
	moostock.c			\
	moouixml.c			\
	mooundo.c			\
	mooutils-fs.c			\
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

nodist_mooutils_sources =		\
	moomarshals.c			\
	moomarshals.h
CLEANFILES =				\
	moomarshals.c			\
	moomarshals.h

EXTRA_DIST =				\
	moomarshals.list		\
	py2h.sh				\
	xml2h.sh			\
	$(gmappedfile_sources)		\
	$(moonewgtk_gtk_2_4_sources)	\
	glade/accelbutton.glade		\
	glade/accelprefs.glade		\
	glade/moologwindow.glade	\
	pixmaps/ggap.png		\
	pixmaps/ggap.ico		\
	pixmaps/gap.png                 \
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
	glib-genmarshal --prefix=_moo_marshal --body \
	       $(srcdir)/moomarshals.list > moomarshals.c
moomarshals.h: moomarshals.list
	glib-genmarshal --prefix=_moo_marshal --header \
	       $(srcdir)/moomarshals.list > moomarshals.h

XML2H = $(srcdir)/xml2h.sh
mooaccelbutton-glade.h: $(srcdir)/glade/accelbutton.glade $(XML2H)
	sh $(XML2H) MOO_ACCEL_BUTTON_GLADE_UI \
	   $(srcdir)/glade/accelbutton.glade > mooaccelbutton-glade.h
mooaccelprefs-glade.h: $(srcdir)/glade/accelprefs.glade $(XML2H)
	sh $(XML2H) MOO_ACCEL_PREFS_GLADE_UI \
	   $(srcdir)/glade/accelprefs.glade > mooaccelprefs-glade.h
moologwindow-glade.h: $(srcdir)/glade/moologwindow.glade $(XML2H)
	sh $(XML2H) MOO_LOG_WINDOW_GLADE_UI \
	   $(srcdir)/glade/moologwindow.glade > moologwindow-glade.h

mooutils_pixmaps =			\
	$(srcdir)/pixmaps/gap.png	\
	$(srcdir)/pixmaps/ggap.png	\
	$(srcdir)/pixmaps/medit.png	\
	$(srcdir)/pixmaps/close.png	\
	$(srcdir)/pixmaps/detach.png	\
	$(srcdir)/pixmaps/attach.png	\
	$(srcdir)/pixmaps/keepontop.png	\
	$(srcdir)/pixmaps/sticky.png

stock-moo.h: $(mooutils_pixmaps)
	gdk-pixbuf-csource --static --build-list			\
		GAP_ICON  $(srcdir)/pixmaps/gap.png			\
		GGAP_ICON $(srcdir)/pixmaps/ggap.png			\
		MEDIT_ICON $(srcdir)/pixmaps/medit.png			\
		MOO_CLOSE_ICON $(srcdir)/pixmaps/close.png		\
		MOO_STICKY_ICON $(srcdir)/pixmaps/sticky.png		\
		MOO_DETACH_ICON $(srcdir)/pixmaps/detach.png		\
		MOO_ATTACH_ICON $(srcdir)/pixmaps/attach.png		\
		MOO_KEEP_ON_TOP_ICON $(srcdir)/pixmaps/keepontop.png	\
			> stock-moo.h

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
	$(MOO_DEBUG_CFLAGS)

if MOO_INSTALL_LIB
mooutils_includedir = $(moo_includedir)/mooutils
mooutils_include_HEADERS = $(mooutils_include_headers)
endif
