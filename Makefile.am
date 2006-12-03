AUTOMAKE_OPTIONS = dist-bzip2 no-dist-gzip foreign nostdinc

m4files =			\
	m4/moo-comps.m4		\
	m4/moo-debug.m4		\
	m4/moo-dirs.m4		\
	m4/moo-fam.m4		\
	m4/moo-flags.m4		\
	m4/moo-funcs.m4		\
	m4/moo-gtk.m4		\
	m4/moo-i18n.m4		\
	m4/moo-os.m4		\
	m4/moo-pcre.m4		\
	m4/moo-progs.m4		\
	m4/moo-pygtk.m4		\
	m4/moo-python.m4	\
	m4/moo-reloc.m4		\
	m4/moo-version.m4	\
	m4/moo-xdgmime.m4	\
	m4/moo-xml.m4

EXTRA_DIST =			\
	$(m4files)		\
	autogen.sh		\
	mingw-configure		\
	cygwin-configure	\
	makewin			\
	COPYING.lib		\
	THANKS			\
	intltool-extract.in	\
	intltool-merge.in	\
	intltool-update.in	\
	mkpotsubdirs		\
	medit.desktop.in

all-am: pot-subdirs-stamp wininstaller/medit.iss m4-stamp

m4-stamp: $(m4files)
	echo stamp > m4-stamp
	for f in $(m4files); do						\
		if test $(srcdir)/$$f -nt $(srcdir)/configure; then	\
			touch $(srcdir)/configure.ac;			\
		fi							\
	done

pot-subdirs-stamp: po/POTFILES.in
	$(SHELL) $(srcdir)/mkpotsubdirs "$(srcdir)"
CLEANFILES = pot-subdirs-stamp

DIST_SUBDIRS = moo tests po
# need to create pot-subdirs-stamp before it goes to po/
SUBDIRS = . moo tests po

ACLOCAL_AMFLAGS = -I m4

@INTLTOOL_DESKTOP_RULE@
DESKTOP_IN_FILES = medit.desktop.in
DESKTOP_FILES =$(DESKTOP_IN_FILES:.desktop.in=.desktop)
desktopdir = $(datadir)/applications
desktop_DATA = $(DESKTOP_FILES)

DISTCLEANFILES =		\
	m4-stamp		\
	configargs.h		\
	$(DESKTOP_FILES)	\
	intltool-extract	\
	intltool-merge		\
	intltool-update

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
