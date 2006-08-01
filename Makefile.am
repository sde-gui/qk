AUTOMAKE_OPTIONS = dist-bzip2 foreign nostdinc

EXTRA_DIST =			\
	autogen.sh		\
	mingw-configure		\
	cygwin-configure	\
	makewin			\
	THANKS

DIST_SUBDIRS = m4 moo tests
SUBDIRS = moo tests

ACLOCAL_AMFLAGS = -I $(top_srcdir)/m4

desktopdir = $(datadir)/applications
desktop_DATA = medit.desktop
appicondir = $(datadir)/pixmaps
appicon_DATA = moo/mooutils/pixmaps/medit.png
hicolor = $(datadir)/icons/hicolor
iconthemedir = $(hicolor)/48x48/apps
icontheme_DATA = moo/mooutils/pixmaps/medit.png
EXTRA_DIST += $(desktop_DATA)

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
