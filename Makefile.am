AUTOMAKE_OPTIONS = dist-bzip2 foreign nostdinc

EXTRA_DIST =			\
	autogen.sh		\
	mingw-configure		\
	cygwin-configure	\
	makewin			\
	THANKS			\
	intltool-extract.in	\
	intltool-merge.in	\
	intltool-update.in	\
	medit.desktop.in

DIST_SUBDIRS = m4 moo tests po
SUBDIRS = moo tests po

ACLOCAL_AMFLAGS = -I $(top_srcdir)/m4

@INTLTOOL_DESKTOP_RULE@
DESKTOP_IN_FILES = medit.desktop.in
DESKTOP_FILES =$(DESKTOP_IN_FILES:.desktop.in=.desktop)
desktopdir = $(datadir)/applications
desktop_DATA = $(DESKTOP_FILES)

appicondir = $(datadir)/pixmaps
appicon_DATA = moo/mooutils/pixmaps/medit.png
hicolor = $(datadir)/icons/hicolor
iconthemedir = $(hicolor)/48x48/apps
icontheme_DATA = moo/mooutils/pixmaps/medit.png

# update_icon_cache = gtk-update-icon-cache -f -t $(hicolor)
#
# install-data-hook:
# 	if echo "Updating icon cache" && $(update_icon_cache); then		\
# 		echo "Done.";							\
# 	else									\
# 		echo "*** GTK icon cache not updated. After install, run this:";\
# 		echo $(update_icon_cache);					\
# 	fi

all-am: wininstaller/medit.iss
