AUTOMAKE_OPTIONS = dist-bzip2 foreign nostdinc

EXTRA_DIST =			\
	autogen.sh		\
	mingw-configure		\
	cygwin-configure	\
	makewin			\
	THANKS

DIST_SUBDIRS = m4

ACLOCAL_AMFLAGS = -I $(top_srcdir)/m4

if MOO_BUILD_PCRE
pcre_include = -I$(top_srcdir)/$(MOO_SRC_PREFIX)/mooutils/pcre
endif

AM_CFLAGS =						\
    -I$(top_builddir)					\
    -I$(top_srcdir)					\
    -I$(top_srcdir)/$(MOO_SRC_PREFIX)			\
    -I$(top_builddir)/$(MOO_SRC_PREFIX)			\
    -I$(top_srcdir)/$(MOO_SRC_PREFIX)/mooutils		\
    -I$(top_builddir)/$(MOO_SRC_PREFIX)/mooutils	\
    $(pcre_include)					\
    $(MOO_CFLAGS)					\
    $(PYTHON_INCLUDES)					\
    $(PYGTK_CFLAGS)					\
    $(MOO_DEBUG_CFLAGS)


###########################################################################

THANKS = $(srcdir)/THANKS
CLEANFILES =
EXTRA_PROGRAMS =
noinst_PROGRAMS =
moo_prefix = moo
XML2H = $(srcdir)/moo/mooutils/xml2h.sh
PY2H = $(srcdir)/moo/mooutils/py2h.sh
SUBDIRS =

# common moo Makefile
include moo/Makefile.incl
# mooutils
include moo/mooutils/Makefile.incl
include moo/mooutils/glade/Makefile.incl
include moo/mooutils/moofileview/Makefile.incl
include moo/mooutils/mooscript/Makefile.incl
include moo/mooutils/newgtk/Makefile.incl
include moo/mooutils/pcre/Makefile.incl
include moo/mooutils/pixmaps/Makefile.incl
include moo/mooutils/xdgmime/Makefile.incl
# mooedit
include moo/mooedit/Makefile.incl
include moo/mooedit/glade/Makefile.incl
include moo/mooedit/syntax/Makefile.incl
include moo/mooedit/plugins/Makefile.incl
include moo/mooedit/plugins/activestrings/Makefile.incl
include moo/mooedit/plugins/completion/Makefile.incl
include moo/mooedit/plugins/fileselector/Makefile.incl
# mooterm
include moo/mooterm/Makefile.incl
include moo/mooterm/glade/Makefile.incl
# mooapp
include moo/mooapp/Makefile.incl
include moo/mooapp/glade/Makefile.incl
# moopython
include moo/moopython/Makefile.incl
include moo/moopython/codegen/Makefile.incl
include moo/moopython/pygtk/Makefile.incl
include moo/moopython/plugins/Makefile.incl

if MOO_BUILD_LIB
moo_libinclude_HEADERS = $(moo_libinclude_headers)
moo_include_HEADERS = $(moo_include_headers)
mooedit_includedir = $(moo_includedir)/mooedit
mooedit_include_HEADERS = $(mooedit_include_headers)
mooterm_includedir = $(moo_includedir)/mooterm
mooterm_include_HEADERS = $(mooterm_include_headers)
mooapp_includedir = $(moo_includedir)/mooapp
mooapp_include_HEADERS = $(mooapp_include_headers)
mooutils_includedir = $(moo_includedir)/mooutils
mooutils_include_HEADERS = $(mooutils_include_headers)
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = moo/moo.pc
endif

if MOO_OS_MINGW
moo_sources += $(moo_win32_sources)
moo_extra_dist += $(moo_unix_sources)
endif
if MOO_OS_UNIX
moo_sources += $(moo_unix_sources)
moo_extra_dist += $(moo_win32_sources)
endif

###########################################################################

CLEANFILES += $(moo_cleanfiles)
BUILT_SOURCES = $(moo_built_sources)

libmoo_la_ldflags = -version-info $(LTVERSION) -no_undefined -export-symbols-regex "^(moo_|ms_|m_|egg_regex_).*"
libmoo_sources = $(moo_sources)
nodist_libmoo_sources = $(moo_nodist_sources)

EXTRA_DIST += $(moo_extra_dist)

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

##############################################################################
# tests
#
include tests/Makefile.incl

generated = 			\
	tests/medit.rc		\
	wininstaller/medit.iss

all-am: $(generated)
