#
#   mooutils/Makefile.am
#

DIST_SUBDIRS = newgtk glade pcre unrtf pixmaps

DEFAULT_INCLUDES =
AM_CFLAGS =                             \
    -I$(top_builddir)                   \
    -I$(top_builddir)/$(MOO_SRC_PREFIX) \
    $(MOO_CFLAGS)                       \
    $(DEBUG_CFLAGS)                     \
    $(CFLAGS)                           \
    $(GTK_CFLAGS)                       \
    $(PYTHON_INCLUDES)                  \
    $(PYGTK_CFLAGS)                     \
    -DG_LOG_DOMAIN=\"Moo\"              \
    -D__MOO__

# if USE_UNRTF
#     unrtf = unrtf
#     unrtf_lib = unrtf/libunrtf.la
#     unrtf_src = moortf.h moortf.c
# endif USE_UNRTF

SUBDIRS = pixmaps glade newgtk pcre $(unrtf) .

noinst_LTLIBRARIES = libmooutils.la

libmooutils_la_LIBADD = \
    pcre/libpcre.la     \
    $(unrtf_lib)        \
    newgtk/libnewgtk.la

libmooutils_la_SOURCES =        \
    $(pcre_src)                 \
    $(unrtf_src)                \
    eggregex.c                  \
    eggregex.h                  \
    mooaccelbutton.c            \
    mooaccelbutton.h            \
    mooaccelbutton-glade.h      \
    mooclosure.c                \
    mooclosure.h                \
    moocompat.c                 \
    moocompat.h                 \
    moodialogs.c                \
    moodialogs.h                \
    moofileutils.c              \
    moofileutils.h              \
    mooglade.c                  \
    mooglade.h                  \
    moolog.c                    \
    moolog.h                    \
    moomarkup.c                 \
    moomarkup.h                 \
    mooprefs.c                  \
    mooprefs.h                  \
    mooprefsdialog.c            \
    mooprefsdialog.h            \
    mooprefsdialogpage.c        \
    mooprefsdialog-settings.c   \
    mooprefsdialog-settings.h   \
    moostock.c                  \
    moostock.h                  \
    mooutils-gobject.c          \
    mooutils-gobject.h          \
    moowin.c                    \
    moowin.h                    \
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

if USE_PYTHON
libmooutils_la_SOURCES += $(python_sources)
endif

nodist_libmooutils_la_SOURCES = \
    moomarshals.c               \
    moomarshals.h
CLEANFILES = $(nodist_libmooutils_la_SOURCES)

EXTRA_DIST =            \
    $(unrtf_src)        \
    $(python_sources)   \
    moomarshals.list    \
    xml2h.sh

BUILT_SOURCES =             \
    mooaccelbutton-glade.h  \
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
