#
#   mooedit/Makefile.am
#

SUBDIRS = glade syntax plugins .


DEFAULT_INCLUDES =
AM_CFLAGS =                             \
    -I$(top_builddir)                   \
    -I$(top_builddir)/$(MOO_SRC_PREFIX) \
    $(MOO_CFLAGS)                       \
    $(MOO_DEBUG_CFLAGS)                 \
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
    moolangmgr.h                \
    moolineview.h               \
    mooplugin-macro.h           \
    mooplugin.h                 \
    mootextbuffer.h             \
    mootextiter.h               \
    mootextstyle.h              \
    mootextstylescheme.h        \
    mootextview.h

noinst_HEADERS =                \
    gtksourceiter.h             \
    mooedit-private.h           \
    mooeditcolorsprefs-glade.h  \
    mooeditdialogs.h            \
    mooeditfileops.h            \
    mooeditfind-glade.h         \
    mooeditgotoline-glade.h     \
    mooeditprefs-glade.h        \
    moohighlighter.h            \
    moolang-aux.h               \
    moolang-parser.h            \
    moolang-rules.h             \
    moolang-strings.h           \
    moolinebuffer.h             \
    moopluginprefs-glade.h      \
    mootextbtree.h              \
    mootextview-private.h


if !MOO_OS_MINGW
unix_sources = moocmdview.c
mooeditinclude_HEADERS += moocmdview.h
endif

libmooedit_la_SOURCES =         \
    $(unix_sources)             \
    $(mooeditinclude_HEADERS)   \
    $(noinst_HEADERS)           \
    gtksourceiter.c             \
    mooedit.c                   \
    mooeditdialogs.c            \
    mooeditfileops.c            \
    mooeditfind.c               \
    mooeditinput.c              \
    mooeditor.c                 \
    mooeditprefs.c              \
    mooeditprefspage.c          \
    mooeditsearch.c             \
    mooeditwindow.c             \
    moohighlighter.c            \
    mooindenter.c               \
    moolang.c                   \
    moolangmgr.c                \
    moolang-parser.c            \
    moolang-rules.c             \
    moolinebuffer.c             \
    moolineview.c               \
    mooplugin.c                 \
    mootextbtree.c              \
    mootextbuffer.c             \
    mootextstyle.c              \
    mootextstylescheme.c        \
    mootextview.c

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
