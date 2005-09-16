#
#   mooedit/Makefile.am
#

DIST_SUBDIRS = glade xdgmime gtksourceview language-specs plugins

if MOO_USE_XDGMIME
    xdgmime = xdgmime
    xdgmime_lib = xdgmime/libxdgmime.la
endif MOO_USE_XDGMIME

SUBDIRS = moofileview glade $(xdgmime) gtksourceview language-specs plugins .


DEFAULT_INCLUDES =
AM_CFLAGS =                             \
    -DXDG_PREFIX=_moo_edit_xdg          \
    -I$(top_builddir)                   \
    -I$(top_builddir)/$(MOO_SRC_PREFIX) \
    $(MOO_CFLAGS)                       \
    $(DEBUG_CFLAGS)                     \
    $(CFLAGS)                           \
    -DG_LOG_DOMAIN=\"Moo\"              \
    -D__MOO__                           \
    -DMOO_MARSHALS_H=\"mooutils/moomarshals.h\"

noinst_LTLIBRARIES = libmooedit.la

libmooedit_la_LIBADD =                  \
    gtksourceview/libgtksourceview.la   \
    moofileview/libmoofileview.la       \
    plugins/libplugins.la               \
    $(xdgmime_lib)

libmooedit_la_SOURCES =     \
    moobigpaned.c           \
    moobigpaned.h           \
    moocmdview.c            \
    moocmdview.h            \
    mooedit.c               \
    mooedit.h               \
    mooedit-private.h       \
    mooeditdialogs.c        \
    mooeditdialogs.h        \
    mooeditfileops.c        \
    mooeditfind.c           \
    mooeditinput.c          \
    mooeditlang.c           \
    mooeditlang.h           \
    mooeditlang-private.h   \
    mooeditlangmgr.c        \
    mooeditlangmgr.h        \
    mooeditor.c             \
    mooeditor.h             \
    mooeditprefs.c          \
    mooeditprefs.h          \
    mooeditprefspage.c      \
    mooeditsearch.c         \
    mooeditsearch.h         \
    mooeditwindow.c         \
    mooeditwindow.h         \
    mooindenter.c           \
    mooindenter.h           \
    moonotebook.c           \
    moonotebook.h           \
    moopaned.c              \
    moopaned.h              \
    moopaneview.c           \
    moopaneview.h           \
    mooplugin.c             \
    mooplugin.h             \
    moorecentmgr.c          \
    moorecentmgr.h          \
    mootextbuffer.c         \
    mootextbuffer.h         \
    mootextiter.h           \
    mootextview.c           \
    mootextview.h           \
    mooeditgotoline-glade.h \
    mooeditfind-glade.h     \
    moopluginprefs-glade.h  \
    mooeditprefs-glade.h

if MOO_USE_XML
    libmooedit_la_SOURCES += mooeditlang-xml.c
endif MOO_USE_XML

EXTRA_DIST =                \
    mooeditlang-xml.c

BUILT_SOURCES =             \
    mooeditgotoline-glade.h \
    mooeditfind-glade.h     \
    moopluginprefs-glade.h  \
    mooeditprefs-glade.h

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

moopluginprefs-glade.h: $(GLADE)/moopluginprefs.glade
	sh $(XML2H) MOO_PLUGIN_PREFS_GLADE_UI $(GLADE)/moopluginprefs.glade \
		> moopluginprefs-glade.h
