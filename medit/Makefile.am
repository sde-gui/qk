AM_CFLAGS =			\
	-I$(top_builddir)	\
	-I../moo		\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)

if MOO_ENABLE_PROJECT
AM_CFLAGS += -DMOO_ENABLE_PROJECT
endif

bin_PROGRAMS = medit

medit_SOURCES = medit-credits.h main.c mem-debug.h
BUILT_SOURCES = medit-credits.h medit.iss
nodist_medit_SOURCES =

medit_LDFLAGS = -export-dynamic $(PYTHON_EXTRA_LDFLAGS) $(medit_res_ldflag)
medit_LDADD = ../moo/libmoo.la

if MOO_OS_MINGW
rc_in_files = medit.rc.in
medit_LDFLAGS += -mwindows
medit_res_ldflag = -Wl,medit.res
nodist_medit_SOURCES += medit.res
endif

EXTRA_DIST =
CLEANFILES =
include ../mk/rc.mk

medit-credits.h: ../THANKS $(XML2H)
	$(SHELL) $(XML2H) THANKS $(srcdir)/../THANKS > $@.tmp && \
	mv $@.tmp $(srcdir)/medit-credits.h

EXTRA_DIST += medit.desktop.in

@MOO_INTLTOOL_DESKTOP_RULE@
DESKTOP_IN_FILES = medit.desktop.in
DESKTOP_FILES =$(DESKTOP_IN_FILES:.desktop.in=.desktop)
desktopdir = $(datadir)/applications
desktop_DATA = $(DESKTOP_FILES)
DISTCLEANFILES = $(DESKTOP_FILES)

appicondir = $(datadir)/pixmaps
appicon_DATA = ../moo/mooutils/pixmaps/medit.png
hicolor = $(datadir)/icons/hicolor
iconthemedir = $(hicolor)/48x48/apps
icontheme_DATA = ../moo/mooutils/pixmaps/medit.png

update_icon_cache = gtk-update-icon-cache -f -t $(DESTDIR)$(hicolor)

if MOO_ENABLE_GENERATED_FILES
install-data-hook:
	if echo "Updating icon cache" && $(update_icon_cache); then		\
		echo "Done.";							\
	else									\
		echo "*** GTK icon cache not updated. After install, run this:";\
		echo $(update_icon_cache);					\
	fi
uninstall-hook:
	if echo "Updating icon cache" && $(update_icon_cache); then echo "Done."; else echo "Failed."; fi
endif

bundle:
	ige-mac-bundler $(srcdir)/mac/medit.bundle
EXTRA_DIST +=		    \
	mac/medit.bundle    \
	mac/Info.plist	    \
	mac/gtkrc	    \
	mac/medit.icns	    \
	mac/environment.sh
