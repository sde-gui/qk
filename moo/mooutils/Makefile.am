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
    -DG_LOG_DOMAIN=\"Moo\"

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
    bind.c                      \
    bind.h                      \
    eggregex.c                  \
    eggregex.h                  \
    mooaccelbutton.c            \
    mooaccelbutton.h            \
    mooclosure.c                \
    mooclosure.h                \
    moocompat.c                 \
    moocompat.h                 \
    moodialogs.c                \
    moodialogs.h                \
    moofileutils.c              \
    moofileutils.h              \
    moolog.c                    \
    moolog.h                    \
    moomarkup.c                 \
    moomarkup.h                 \
    mooobjectfactory.c          \
    mooobjectfactory.h          \
    moopaned.c                  \
    moopaned.h                  \
    mooparam.c                  \
    mooparam.h                  \
    mooprefs.c                  \
    mooprefs.h                  \
    mooprefsdialog.c            \
    mooprefsdialog.h            \
    mooprefsdialogpage.c        \
    mooprefsdialog-settings.c   \
    mooprefsdialog-settings.h   \
    moosignal.c                 \
    moosignal.h                 \
    moostock.c                  \
    moostock.h                  \
    mootypes.c                  \
    mootypes.h                  \
    moowin.c                    \
    moowin.h                    \
    shortcutdialog-glade.c      \
    stock-about-16.h            \
    stock-about-24.h            \
    stock-moo.h                 \
    stock-edit-16.h             \
    stock-edit-24.h             \
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

if GTK_2_4
libmooutils_la_SOURCES +=   \
    moofilechooser.c        \
    moofilechooser.h
endif GTK_2_4

EXTRA_DIST =            \
    $(unrtf_src)        \
    $(python_sources)   \
    moofilechooser.c    \
    moofilechooser.h    \
    moomarshals.list    \
    xml2h.sh

BUILT_SOURCES =             \
    moomarshals.h           \
    moomarshals.c           \
    shortcutdialog-glade.c  \
    stock-moo.h

moomarshals.c: moomarshals.list
	glib-genmarshal --prefix=_moo_marshal --body \
	       $(srcdir)/moomarshals.list > moomarshals.c
moomarshals.h: moomarshals.list
	glib-genmarshal --prefix=_moo_marshal --header \
	       $(srcdir)/moomarshals.list > moomarshals.h

GLADE = $(srcdir)/glade
shortcutdialog-glade.c: $(GLADE)/shortcutdialog.c $(GLADE)/shortcutdialog.sed $(SUPPORT)
	cat $(GLADE)/shortcutdialog.c | sh $(GLADE)/support.sh | \
		sed -f $(GLADE)/shortcutdialog.sed > shortcutdialog-glade.c

PIXMAPS =                           \
    $(srcdir)/pixmaps/gap.png       \
    $(srcdir)/pixmaps/ggap.png      \
    $(srcdir)/pixmaps/close.png     \
    $(srcdir)/pixmaps/sticky.png

stock-moo.h: $(PIXMAPS)
	gdk-pixbuf-csource --static --build-list              \
		GAP_ICON  $(srcdir)/pixmaps/gap.png           \
		GGAP_ICON $(srcdir)/pixmaps/ggap.png          \
		MOO_CLOSE_ICON $(srcdir)/pixmaps/close.png    \
		MOO_STICKY_ICON $(srcdir)/pixmaps/sticky.png  \
	> $(srcdir)/stock-moo.h
