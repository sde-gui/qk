#
#   mooedit/Makefile.am
#

SUBDIRS = glade syntax plugins .


DEFAULT_INCLUDES =
AM_CFLAGS =                             \
    -I$(top_builddir)                   \
    -I$(top_builddir)/$(MOO_SRC_PREFIX) \
    $(MOO_CFLAGS)                       \
    $(DEBUG_CFLAGS)                     \
    $(CFLAGS)

noinst_LTLIBRARIES = libmooedit.la

mooeditincludedir = $(includedir)/moo-1.0/mooedit
mooeditinclude_HEADERS =        \
    mooedit.h                   \
    mooeditor.h                 \
    mooeditprefs.h              \
    mooeditsearch.h             \
    mooeditwindow.h             \
    mooindenter.h               \
    moolang.h                   \
    moolineview.h               \
    mooplugin.h                 \
    mooplugin-macro.h           \
    mootextbuffer.h             \
    mootextiter.h               \
    mootextstyle.h              \
    mootextview.h


if !MOO_OS_MINGW
unix_sources =              \
    moocmdview.c            \
    moocmdview.h
mooeditinclude_HEADERS += moocmdview.h
endif

libmooedit_la_SOURCES =         \
    $(unix_sources)             \
    $(mooeditinclude_HEADERS)   \
    gtksourceiter.c             \
    gtksourceiter.h             \
    mooedit.c                   \
    mooedit-private.h           \
    mooeditdialogs.c            \
    mooeditdialogs.h            \
    mooeditfileops.c            \
    mooeditfileops.h            \
    mooeditfind.c               \
    mooeditinput.c              \
    mooeditor.c                 \
    mooeditprefs.c              \
    mooeditprefs.h              \
    mooeditprefspage.c          \
    mooeditsearch.c             \
    mooeditsearch.h             \
    mooeditwindow.c             \
    mooeditwindow.h             \
    moohighlighter.c            \
    moohighlighter.h            \
    mooindenter.c               \
    mooindenter.h               \
    moolang.c                   \
    moolang.h                   \
    moolang-parser.c            \
    moolang-parser.h            \
    moolang-rules.c             \
    moolang-rules.h             \
    moolang-aux.h               \
    moolang-strings.h           \
    moolinebuffer.c             \
    moolinebuffer.h             \
    moolineview.c               \
    moolineview.h               \
    mooplugin.c                 \
    mooplugin.h                 \
    mooplugin-macro.h           \
    mootextbtree.c              \
    mootextbtree.h              \
    mootextbuffer.c             \
    mootextbuffer.h             \
    mootextiter.h               \
    mootextstyle.c              \
    mootextstyle.h              \
    mootextview.c               \
    mootextview.h               \
    mootextview-private.h       \
    mooeditgotoline-glade.h     \
    mooeditfind-glade.h         \
    moopluginprefs-glade.h      \
    mooeditprefs-glade.h        \
    mooeditcolorsprefs-glade.h

libmooedit_la_LIBADD =      \
    plugins/libplugins.la

BUILT_SOURCES =             \
    mooeditgotoline-glade.h \
    mooeditfind-glade.h     \
    moopluginprefs-glade.h  \
    mooeditprefs-glade.h    \
    mooeditcolorsprefs-glade.h


GLADE = $(srcdir)/glade
XML2H = $(srcdir)/../mooutils/xml2h.sh

mooeditgotoline-glade.h: $(GLADE)/mooeditgotoline.glade
	sh $(XML2H) MOO_EDIT_GOTO_LINE_GLADE_UI $(GLADE)/mooeditgotoline.glade \
		> mooeditgotoline-glade.h

mooeditfind-glade.h: $(GLADE)/mooeditfind.glade
	sh $(XML2H) MOO_EDIT_FIND_GLADE_UI $(GLADE)/mooeditfind.glade \
		> mooeditfind-glade.h

mooeditprefs-glade.h: $(GLADE)/mooeditprefs.glade
	sh $(XML2H) MOO_EDIT_PREFS_GLADE_UI $(GLADE)/mooeditprefs.glade \
		> mooeditprefs-glade.h

mooeditcolorsprefs-glade.h: $(GLADE)/mooeditcolorsprefs.glade
	sh $(XML2H) MOO_EDIT_COLORS_PREFS_GLADE_UI $(GLADE)/mooeditcolorsprefs.glade \
		> mooeditcolorsprefs-glade.h

moopluginprefs-glade.h: $(GLADE)/moopluginprefs.glade
	sh $(XML2H) MOO_PLUGIN_PREFS_GLADE_UI $(GLADE)/moopluginprefs.glade \
		> moopluginprefs-glade.h
