#
#   mooedit/Makefile.am
#

DIST_SUBDIRS = glade xdgmime gtksourceview language-specs

if MOO_USE_XDGMIME
    xdgmime = xdgmime
    xdgmime_lib = xdgmime/libxdgmime.la
endif MOO_USE_XDGMIME

SUBDIRS = moofileview glade $(xdgmime) gtksourceview language-specs .


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

noinst_LTLIBRARIES   = libmooedit.la

libmooedit_la_LIBADD =                  \
    gtksourceview/libgtksourceview.la   \
    moofileview/libmoofileview.la       \
    $(xdgmime_lib)

libmooedit_la_SOURCES =     \
    moobigpaned.c           \
    moobigpaned.h           \
    mooedit.c               \
    mooedit.h               \
    mooedit-private.h       \
    mooeditcursor.c         \
    mooeditdialogs.c        \
    mooeditdialogs.h        \
    mooeditfileops.c        \
    mooeditfind.c           \
    mooeditfind-glade.c     \
    mooeditgotoline-glade.c \
    mooeditkeys.c           \
    mooeditlang.c           \
    mooeditlang.h           \
    mooeditlang-private.h   \
    mooeditlangmgr.c        \
    mooeditlangmgr.h        \
    mooeditor.c             \
    mooeditor.h             \
    mooeditplugin.c         \
    mooeditplugin.h         \
    mooeditprefs.c          \
    mooeditprefs.h          \
    mooeditprefs-glade.c    \
    mooeditprefspage.c      \
    mooeditsearch.c         \
    mooeditsearch.h         \
    mooeditwindow.c         \
    mooeditwindow.h         \
    moonotebook.c           \
    moonotebook.h           \
    moopaned.c              \
    moopaned.h              \
    moorecentmgr.c          \
    moorecentmgr.h          \
    mootextiter.h

if MOO_USE_XML
    libmooedit_la_SOURCES += mooeditlang-xml.c
endif MOO_USE_XML

EXTRA_DIST =                \
    mooeditlang-xml.c

BUILT_SOURCES =             \
    mooeditgotoline-glade.c \
    mooeditfind-glade.c     \
    mooeditprefs-glade.c

GLADE = $(srcdir)/glade
TOOLBAR = $(top_srcdir)/$(MOO_SRC_PREFIX)/mooutils/glade/toolbar.sh
SUPPORT = $(top_srcdir)/$(MOO_SRC_PREFIX)/mooutils/glade/support.sh

mooeditfind-glade.c: $(GLADE)/mooeditfind.c $(GLADE)/mooeditfind.sed $(TOOLBAR) $(SUPPORT)
	cat $(GLADE)/mooeditfind.c | sh $(SUPPORT) | sed -f $(GLADE)/mooeditfind.sed \
		> mooeditfind-glade.c

mooeditgotoline-glade.c: $(GLADE)/mooeditgotoline.c $(GLADE)/mooeditgotoline.sed $(TOOLBAR) $(SUPPORT)
	cat $(GLADE)/mooeditgotoline.c | sh $(SUPPORT) | sed -f $(GLADE)/mooeditgotoline.sed \
		> mooeditgotoline-glade.c

mooeditprefs-glade.c: $(GLADE)/mooeditprefs.c $(GLADE)/mooeditprefs.sed $(TOOLBAR) $(SUPPORT)
	cat  $(GLADE)/mooeditprefs.c | sh $(SUPPORT) | \
	sed -f $(GLADE)/mooeditprefs.sed > mooeditprefs-glade.c
