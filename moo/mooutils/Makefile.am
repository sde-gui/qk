#
#   mooutils/Makefile.am
#

DIST_SUBDIRS = newgtk glade pcre pixmaps moofileview xdgmime

if MOO_USE_XDGMIME
    xdgmime = xdgmime
    xdgmime_lib = xdgmime/libxdgmime.la
endif MOO_USE_XDGMIME

if !MOO_OS_MINGW
    moofileview = moofileview
    moofileview_lib = moofileview/libmoofileview.la
endif !MOO_OS_MINGW

DEFAULT_INCLUDES =
AM_CFLAGS =                             \
    -I$(top_builddir)                   \
    -I$(top_builddir)/$(MOO_SRC_PREFIX) \
    $(MOO_CFLAGS)                       \
    $(MOO_DEBUG_CFLAGS)                 \
    $(CFLAGS)                           \
    $(PYTHON_INCLUDES)                  \
    $(PYGTK_CFLAGS)

SUBDIRS = pixmaps glade newgtk pcre $(moofileview) $(xdgmime) .

noinst_LTLIBRARIES = libmooutils.la

libmooutils_la_LIBADD =             \
    pcre/libpcre.la                 \
    newgtk/libnewgtk.la             \
    $(moofileview_lib)              \
    $(xdgmime_lib)

utilsinclude_headers =          \
    eggregex.h                  \
    mooaccelbutton.h            \
    moobigpaned.h               \
    moocellrenderercolor.h      \
    mooclosure.h                \
    moocombo.h                  \
    mooentry.h                  \
    moofiltermgr.h              \
    mooglade.h                  \
    moohistoryentry.h           \
    moohistorylist.h            \
    moomarkup.h                 \
    moomenumgr.h                \
    moonotebook.h               \
    moopaned.h                  \
    mooprefs.h                  \
    mooprefsdialog.h            \
    mooprefsdialogpage.h        \
    moostock.h                  \
    mooutils-gobject.h          \
    mooaccel.h                  \
    mooaction.h                 \
    mooactiongroup.h            \
    moomenuaction.h             \
    mootoggleaction.h           \
    moouixml.h                  \
    mooutils-misc.h             \
    moowindow.h                 \
    mooutils-python.h

win32_sources =                 \
    moohandlewatch.h            \
    moohandlewatch.c

if MOO_STANDALONE
utilsincludedir = $(includedir)/moo-1.0/mooutils
utilsinclude_HEADERS = $(utilsinclude_headers)
endif MOO_STANDALONE

libmooutils_la_SOURCES =        \
    $(utilsinclude_headers)     \
    eggregex.c                  \
    mooaccelbutton.c            \
    mooaccelbutton-glade.h      \
    moobigpaned.c               \
    moocellrenderercolor.c      \
    mooclosure.c                \
    moocombo.c                  \
    moocompat.c                 \
    moocompat.h                 \
    moodialogs.c                \
    moodialogs.h                \
    mooentry.c                  \
    moofilewatch.c              \
    moofilewatch.h              \
    moofiltermgr.c              \
    mooglade.c                  \
    moohistoryentry.c           \
    moohistorylist.c            \
    moomarkup.c                 \
    moomenumgr.c                \
    moonotebook.c               \
    moopaned.c                  \
    mooprefs.c                  \
    mooprefsdialog.c            \
    mooprefsdialogpage.c        \
    moostock.c                  \
    mooutils-gobject.c          \
    mooutils-misc.c             \
    mooaccel.c                  \
    mooaccelprefs-glade.h       \
    mooaction.c                 \
    mooactiongroup.c            \
    moomenuaction.c             \
    mootoggleaction.c           \
    moouixml.c                  \
    moowindow.c                 \
    gtksourceundomanager.c      \
    gtksourceundomanager.h      \
    stock-about-16.h            \
    stock-about-24.h            \
    stock-moo.h                 \
    stock-edit-16.h             \
    stock-edit-24.h             \
    stock-select-all-16.h       \
    stock-select-all-24.h       \
    stock-terminal-24.h

python_sources =                \
    mooutils-python.c           \
    mooutils-python.h

if MOO_USE_PYTHON
libmooutils_la_SOURCES += $(python_sources)
endif

if MOO_OS_MINGW
libmooutils_la_SOURCES += $(win32_sources)
endif

nodist_libmooutils_la_SOURCES = \
    moomarshals.c               \
    moomarshals.h
CLEANFILES = $(nodist_libmooutils_la_SOURCES)

EXTRA_DIST =            \
    $(python_sources)   \
    $(win32_sources)    \
    moomarshals.list    \
    xml2h.sh

BUILT_SOURCES =             \
    mooaccelbutton-glade.h  \
    mooaccelprefs-glade.h   \
    moomarshals.h           \
    moomarshals.c           \
    stock-moo.h

moomarshals.c: moomarshals.list
	glib-genmarshal --prefix=_moo_marshal --body \
	       $(srcdir)/moomarshals.list > moomarshals.c
moomarshals.h: moomarshals.list
	glib-genmarshal --prefix=_moo_marshal --header \
	       $(srcdir)/moomarshals.list > moomarshals.h

mooaccelbutton-glade.h: $(srcdir)/glade/shortcutdialog.glade $(srcdir)/xml2h.sh
	sh $(srcdir)/xml2h.sh MOO_ACCEL_BUTTON_GLADE_UI \
	   $(srcdir)/glade/shortcutdialog.glade > mooaccelbutton-glade.h
mooaccelprefs-glade.h: $(srcdir)/glade/shortcutsprefs.glade $(srcdir)/../mooutils/xml2h.sh
	sh $(srcdir)/xml2h.sh MOO_SHORTCUTS_PREFS_GLADE_UI \
	   $(srcdir)/glade/shortcutsprefs.glade > mooaccelprefs-glade.h

PIXMAPS =                           \
    $(srcdir)/pixmaps/gap.png       \
    $(srcdir)/pixmaps/ggap.png      \
    $(srcdir)/pixmaps/close.png     \
    $(srcdir)/pixmaps/detach.png    \
    $(srcdir)/pixmaps/attach.png    \
    $(srcdir)/pixmaps/keepontop.png \
    $(srcdir)/pixmaps/sticky.png

stock-moo.h: $(PIXMAPS)
	gdk-pixbuf-csource --static --build-list                      \
		GAP_ICON  $(srcdir)/pixmaps/gap.png                   \
		GGAP_ICON $(srcdir)/pixmaps/ggap.png                  \
		MOO_CLOSE_ICON $(srcdir)/pixmaps/close.png            \
		MOO_STICKY_ICON $(srcdir)/pixmaps/sticky.png          \
		MOO_DETACH_ICON $(srcdir)/pixmaps/detach.png          \
		MOO_ATTACH_ICON $(srcdir)/pixmaps/attach.png          \
		MOO_KEEP_ON_TOP_ICON $(srcdir)/pixmaps/keepontop.png  \
	> $(srcdir)/stock-moo.h
