noinst_LTLIBRARIES = libmooutils.la

AM_CPPFLAGS = $(MOO_CPPFLAGS) -I.. -I$(top_srcdir)/doc
AM_CFLAGS = $(MOO_CFLAGS)
AM_CXXFLAGS = $(MOO_CXXFLAGS)

BUILT_SOURCES =

libmooutils_la_SOURCES =	\
	moocpp-types.h		\
	moocpp.cpp		\
	moocpp.h		\
	moocpp-cont.h		\
	moocpp-exc.h		\
	moocpp-macros.h		\
	moocpp-refptr.h		\
	mooutils-thread.c	\
	mooutils-thread.h	\
	mdhistorymgr.c		\
	mdhistorymgr.h		\
	moo-environ.h		\
	mooaccel.c		\
	mooaccel.h		\
	mooaccelbutton.c	\
	mooaccelbutton.h	\
	mooaccelprefs.c		\
	mooaccelprefs.h		\
	mooaction-private.h	\
	mooaction.c		\
	mooaction.h		\
	mooactionbase-private.h	\
	mooactionbase.c		\
	mooactionbase.h		\
	mooactioncollection.c	\
	mooactioncollection.h	\
	mooactionfactory.c	\
	mooactionfactory.h	\
	mooactiongroup.c	\
	mooactiongroup.h	\
	mooapp-ipc.c		\
	mooapp-ipc.h		\
	mooappinput.c		\
	mooappinput.h		\
	mooatom.h		\
	moobigpaned.c		\
	moobigpaned.h		\
	mooclosure.c		\
	mooclosure.h		\
	moocombo.c		\
	moocombo.h		\
	moocompat.c		\
	moocompat.h		\
	moodialogs.c		\
	moodialogs.h		\
	mooeditops.c		\
	mooeditops.h		\
	mooencodings-data.h	\
	mooencodings.c		\
	mooencodings.h		\
	mooentry.c		\
	mooentry.h		\
	moofiledialog.c		\
	moofiledialog.h		\
	moofileicon.c		\
	moofileicon.h		\
	moofilewatch.c		\
	moofilewatch.h		\
	moofiltermgr.c		\
	moofiltermgr.h		\
	moofontsel.c		\
	moofontsel.h		\
	mooglade.c		\
	mooglade.h		\
	moohelp.c		\
	moohelp.h		\
	moohistorycombo.c	\
	moohistorycombo.h	\
	moohistorylist.c	\
	moohistorylist.h	\
	mooi18n.c		\
	mooi18n.h		\
	moolist.h		\
	moomarkup.c		\
	moomarkup.h		\
	moomenu.c		\
	moomenu.h		\
	moomenuaction.c		\
	moomenuaction.h		\
	moomenumgr.c		\
	moomenumgr.h		\
	moomenutoolbutton.c	\
	moomenutoolbutton.h	\
	moo-mime.c		\
	moo-mime.h		\
	moonotebook.c		\
	moonotebook.h		\
	mooonce.h		\
	moopane.c		\
	moopane.h		\
	moopaned.c		\
	moopaned.h		\
	mooprefs.c		\
	mooprefs.h		\
	mooprefsdialog.c	\
	mooprefsdialog.h	\
	mooprefspage.c		\
	mooprefspage.h		\
	moopython.h		\
	moospawn.c		\
	moospawn.h		\
	moostat.h		\
	moostock.c		\
	moostock.h		\
	mootype-macros.h	\
	moouixml.c		\
	moouixml.h		\
	mooundo.c		\
	mooundo.h		\
	mooutils-debug.h	\
	mooutils-enums.c	\
	mooutils-enums.h	\
	mooutils-file.c		\
	mooutils-file.h		\
	mooutils-file-private.h	\
	mooutils-fli.c		\
	mooutils-fs.c		\
	mooutils-fs.h		\
	mooutils-gobject-private.h \
	mooutils-gobject.c	\
	mooutils-gobject.h	\
	mooutils-macros.h	\
	mooutils-mem.h		\
	mooutils-messages.h	\
	mooutils-misc.c		\
	mooutils-misc.h		\
	mooutils-tests.h	\
	mooutils-treeview.c	\
	mooutils-treeview.h	\
	moowindow.c		\
	moowindow.h		\
	stock-file-24.h		\
	stock-file-selector-24.h\
	stock-select-all-16.h	\
	stock-select-all-24.h	\
	stock-terminal-24.h

if MOO_OS_WIN32
libmooutils_la_SOURCES += mooutils-win32.c
endif
if MOO_ENABLE_UNIT_TESTS
libmooutils_la_SOURCES +=	\
	moo-test-utils.c	\
	moo-test-utils.h	\
	moo-test-macros.h
endif

EXTRA_DIST =				\
	glade/mooaccelprefs.glade	\
	glade/mooaccelbutton.glade	\
	glade/moologwindow.glade
BUILT_SOURCES +=			\
	glade/mooaccelprefs-gxml.h	\
	glade/mooaccelbutton-gxml.h	\
	glade/moologwindow-gxml.h

BUILT_SOURCES += stock-moo.h moofileicon-symlink.h
PIXMAPS = pixmaps/hide.png pixmaps/close.png pixmaps/sticky.png pixmaps/detach.png pixmaps/attach.png pixmaps/keepontop.png pixmaps/medit.png
EXTRA_DIST += $(PIXMAPS)
stock-moo.h: $(PIXMAPS)
	$(GDK_PIXBUF_CSOURCE) --static --build-list \
		MOO_HIDE_ICON $(srcdir)/pixmaps/hide.png \
		MOO_CLOSE_ICON $(srcdir)/pixmaps/close.png \
		MOO_STICKY_ICON $(srcdir)/pixmaps/sticky.png \
		MOO_DETACH_ICON $(srcdir)/pixmaps/detach.png \
		MOO_ATTACH_ICON $(srcdir)/pixmaps/attach.png \
		MOO_KEEP_ON_TOP_ICON $(srcdir)/pixmaps/keepontop.png \
		MEDIT_ICON $(srcdir)/pixmaps/medit.png \
			> $@.tmp && mv $@.tmp $@
EXTRA_DIST += pixmaps/symlink.png pixmaps/symlink-small.png
moofileicon-symlink.h: pixmaps/symlink.png pixmaps/symlink-small.png
	$(GDK_PIXBUF_CSOURCE) --static --build-list \
		SYMLINK_ARROW $(srcdir)/pixmaps/symlink.png \
		SYMLINK_ARROW_SMALL $(srcdir)/pixmaps/symlink-small.png \
			> $@.tmp && mv $@.tmp $@

# SET(moo_utils_enum_headers
#   moofiledialog.h
#   moofilewatch.h
#   mooprefs.h
#   moouixml.h
#   mooutils-misc.h
# )

include ../../tools/rules.mk

# -%- strip:true -%-
