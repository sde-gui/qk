#
#   mooui/Makefile.am
#

SUBDIRS = glade .

DEFAULT_INCLUDES =
AM_CFLAGS =                             \
    -I$(top_builddir)                   \
    -I$(top_builddir)/$(MOO_SRC_PREFIX) \
    $(MOO_CFLAGS)                       \
    $(PYTHON_INCLUDES)                  \
    $(PYGTK_CFLAGS)                     \
    $(M_CFLAGS)                         \
    $(CFLAGS)                           \
    $(XML_CFLAGS)                       \
    -DG_LOG_DOMAIN=\"Moo\"

noinst_LTLIBRARIES   = libmooui.la

libmooui_la_SOURCES =           \
    mooaction.c                 \
    mooaction.h                 \
    mooactiongroup.c            \
    mooactiongroup.h            \
    moomenuaction.c             \
    moomenuaction.h             \
    mooshortcutsprefs-glade.c   \
    mooshortcutsprefs.c         \
    mooshortcutsprefs.h         \
    mootext.c                   \
    mootext.h                   \
    mootoggleaction.c           \
    mootoggleaction.h           \
    moouiobject.c               \
    moouiobject.h               \
    moouiobject-impl.h          \
    moouixml.c                  \
    moouixml.h                  \
    moowindow.c                 \
    moowindow.h

EXTRA_DIST =                    \
    moouimanager.cpp            \
    moouimanager.h

BUILT_SOURCES = mooshortcutsprefs-glade.c

GLADE = $(srcdir)/glade
SUPPORT = $(top_srcdir)/$(MOO_SRC_PREFIX)/mooutils/glade/support.sh
mooshortcutsprefs-glade.c: $(GLADE)/shortcutsprefs.c $(GLADE)/shortcutsprefs.sed $(SUPPORT)
	cat $(GLADE)/shortcutsprefs.c | sh $(SUPPORT) | \
		sed -f $(GLADE)/shortcutsprefs.sed > mooshortcutsprefs-glade.c

##############################################################################
# Pygtk stuff
#

EXTRA_DIST += mooui-mod.c mooui-pygtk.defs mooui-pygtk.override

if USE_PYGTK
libmooui_la_SOURCES += mooui-mod.c

nodist_libmooui_la_SOURCES = mooui-pygtk.c
BUILT_SOURCES += mooui-pygtk.c
CLEANFILES = mooui-pygtk.c

mooui-pygtk.c: $(srcdir)/mooui-pygtk.defs $(srcdir)/mooui-pygtk.override
	pygtk-codegen-2.0 --prefix moo_ui \
	--register $(PYGTK_DEFS_DIR)/gtk-types.defs \
	--register $(PYGTK_DEFS_DIR)/gdk-types.defs \
	--override $(srcdir)/mooui-pygtk.override \
	$(srcdir)/mooui-pygtk.defs > mooui-pygtk.c

endif USE_PYGTK
