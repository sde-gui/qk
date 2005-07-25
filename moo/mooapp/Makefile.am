#
#   mooapp/Makefile.am
#

SUBDIRS = glade

DEFAULT_INCLUDES =
AM_CFLAGS =                             \
    -I$(top_builddir)                   \
    -I$(top_builddir)/$(MOO_SRC_PREFIX) \
    $(PYTHON_INCLUDES)                  \
    $(PYGTK_CFLAGS)                     \
    $(MOO_CFLAGS)                       \
    $(DEBUG_CFLAGS)                     \
    $(CFLAGS)                           \
    -DG_LOG_DOMAIN=\"Moo\"

noinst_LTLIBRARIES   = libmooapp.la

BUILT_SOURCES =
libmooapp_la_SOURCES =      \
    mooappdialogs.c         \
    mooapp-private.h        \
    mooapp.c                \
    mooapp.h

if USE_PYTHON
libmooapp_la_SOURCES += \
    mooapp-python.h     \
    mooappinput.c       \
    mooappinput.h       \
    moopythonconsole.c  \
    moopythonconsole.h  \
    moopython.c         \
    moopython.h         \
    pyutils.h

endif USE_PYTHON

EXTRA_DIST =            \
    gpl                 \
    mooappinput.c       \
    mooappinput.h       \
    moopython.c         \
    moopython.h         \
    moopythonconsole.c  \
    moopythonconsole.h  \
    pyutils.h

##############################################################################
# Pygtk stuff
#

EXTRA_DIST += mooapp-mod.c mooapp-pygtk.defs mooapp-pygtk.override

if USE_PYGTK
libmooapp_la_SOURCES += mooapp-mod.c

nodist_libmooapp_la_SOURCES = mooapp-pygtk.c
BUILT_SOURCES += mooapp-pygtk.c
CLEANFILES = mooapp-pygtk.c

mooapp-pygtk.c: $(srcdir)/mooapp-pygtk.defs $(srcdir)/mooapp-pygtk.override
	pygtk-codegen-2.0 --prefix moo_app \
	--register $(PYGTK_DEFS_DIR)/gtk-types.defs \
	--register $(PYGTK_DEFS_DIR)/gdk-types.defs \
	--register $(srcdir)/../mooedit/mooedit-pygtk.defs \
	--register $(srcdir)/../mooui/mooui-pygtk.defs \
	--override $(srcdir)/mooapp-pygtk.override \
	--outfilename mooapp-pygtk.c \
	$(srcdir)/mooapp-pygtk.defs > mooapp-pygtk.c

endif


if GTK_2_6
libmooapp_la_SOURCES += mooappabout-glade.c
BUILT_SOURCES += mooappabout-glade.c
GLADE = $(srcdir)/glade
SUPPORT = $(top_srcdir)/$(MOO_SRC_PREFIX)/mooutils/glade/support.sh
mooappabout-glade.c: $(GLADE)/mooappabout.c $(GLADE)/mooappabout.sed $(SUPPORT)
	cat $(GLADE)/mooappabout.c | sh $(SUPPORT) | sed -f $(GLADE)/mooappabout.sed \
		> mooappabout-glade.c
endif
