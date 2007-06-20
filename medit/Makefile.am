AM_CFLAGS =			\
	-I$(top_builddir)	\
	-I../moo		\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)

EXTRA_DIST =		\
	medit.1		\
	medit-app.opag	\
	medit.rc.in

man_MANS = medit.1

all-am: medit.iss

bin_PROGRAMS = medit

medit_SOURCES = medit-credits.h medit-app.c mem-debug.h
BUILT_SOURCES = medit-credits.h medit-app.c
nodist_medit_SOURCES =

medit_LDFLAGS = -export-dynamic $(PYTHON_EXTRA_LDFLAGS) $(medit_res_ldflag)
medit_LDADD = ../moo/libmoo.la

if MOO_OS_MINGW
medit_LDFLAGS += -mwindows
CLEANFILES = medit.res
@MOO_WIN32_RC_RULE@
medit_res_ldflag = -Wl,medit.res
BUILT_SOURCES += medit.res
nodist_medit_SOURCES += medit.res
endif

XML2H = $(srcdir)/../moo/mooutils/xml2h.sh
medit-credits.h: ../THANKS $(XML2H)
	$(SHELL) $(XML2H) THANKS $(srcdir)/../THANKS > $@.tmp && \
	mv $@.tmp $@

medit-app.c: medit-app.opag
	if test -z $(OPAG); then \
		touch $(srcdir)/medit-app.c; \
	else \
		$(OPAG) -f _medit_parse_options -O _medit_opt_ -A _medit_arg_ $(srcdir)/medit-app.opag $(srcdir)/medit-app.c; \
	fi


EXTRA_DIST += medit.desktop.in

@INTLTOOL_DESKTOP_RULE@
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

install-data-hook:
	if echo "Updating icon cache" && $(update_icon_cache); then		\
		echo "Done.";							\
	else									\
		echo "*** GTK icon cache not updated. After install, run this:";\
		echo $(update_icon_cache);					\
	fi
uninstall-hook:
	echo "Updating icon cache" && $(update_icon_cache) && echo "Done."
