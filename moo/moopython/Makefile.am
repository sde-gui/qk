#
#   moopython/Makefile.am
#

DEFAULT_INCLUDES =
AM_CFLAGS =                                 \
    -I$(top_builddir)                       \
    -I$(top_builddir)/$(MOO_SRC_PREFIX)     \
    $(MOO_CFLAGS)                           \
    $(MOO_PYTHON_DEBUG_CFLAGS)              \
    $(CFLAGS)                               \
    $(GTK_CFLAGS)                           \
    $(PYTHON_INCLUDES)                      \
    $(PYGTK_CFLAGS)


noinst_LTLIBRARIES = libmoopython.la

libmoopython_la_SOURCES = moo-pygtk.c

EXTRA_DIST =                \
    mooapp-mod.c            \
    mooapp-pygtk.defs       \
    mooapp-pygtk.override   \
    mooedit-mod.c           \
    mooedit-pygtk.defs      \
    mooedit-pygtk.override  \
    mooterm-mod.c           \
    mooterm-pygtk.defs      \
    mooterm-pygtk.override  \
    mooutils-mod.c          \
    mooutils-pygtk.defs     \
    mooutils-pygtk.override

nodist_libmoopython_la_SOURCES =
BUILT_SOURCES =
CLEANFILES =

if MOO_BUILD_UTILS
libmoopython_la_SOURCES += mooutils-mod.c
nodist_libmoopython_la_SOURCES += mooutils-pygtk.c
BUILT_SOURCES += mooutils-pygtk.c
CLEANFILES += mooutils-pygtk.c
endif MOO_BUILD_UTILS

if MOO_BUILD_TERM
libmoopython_la_SOURCES += mooterm-mod.c
nodist_libmoopython_la_SOURCES += mooterm-pygtk.c
BUILT_SOURCES += mooterm-pygtk.c
CLEANFILES += mooterm-pygtk.c
endif MOO_BUILD_TERM

if MOO_BUILD_EDIT
libmoopython_la_SOURCES += mooedit-mod.c
nodist_libmoopython_la_SOURCES += mooedit-pygtk.c
BUILT_SOURCES += mooedit-pygtk.c
CLEANFILES += mooedit-pygtk.c
endif MOO_BUILD_EDIT

if MOO_BUILD_APP
libmoopython_la_SOURCES += mooapp-mod.c
nodist_libmoopython_la_SOURCES += mooapp-pygtk.c
BUILT_SOURCES += mooapp-pygtk.c
CLEANFILES += mooapp-pygtk.c
endif MOO_BUILD_APP

mooutils-pygtk.c: $(srcdir)/mooutils-pygtk.defs $(srcdir)/mooutils-pygtk.override
	pygtk-codegen-2.0 --prefix moo_utils \
	--register $(PYGTK_DEFS_DIR)/gtk-types.defs \
	--register $(PYGTK_DEFS_DIR)/gdk-types.defs \
	--override $(srcdir)/mooutils-pygtk.override \
	--outfilename mooutils-pygtk.c \
	$(srcdir)/mooutils-pygtk.defs > mooutils-pygtk.c

mooterm-pygtk.c: $(srcdir)/mooterm-pygtk.defs $(srcdir)/mooterm-pygtk.override
	pygtk-codegen-2.0 --prefix moo_term \
	--register $(PYGTK_DEFS_DIR)/gtk-types.defs \
	--register $(PYGTK_DEFS_DIR)/gdk-types.defs \
	--register $(srcdir)/mooutils-pygtk.defs \
	--override $(srcdir)/mooterm-pygtk.override \
	--outfilename mooterm-pygtk.c \
	$(srcdir)/mooterm-pygtk.defs > mooterm-pygtk.c

mooedit-pygtk.c: $(srcdir)/mooedit-pygtk.defs $(srcdir)/mooedit-pygtk.override
	pygtk-codegen-2.0 --prefix moo_edit \
	--register $(PYGTK_DEFS_DIR)/gtk-types.defs \
	--register $(PYGTK_DEFS_DIR)/gdk-types.defs \
	--register $(srcdir)/mooutils-pygtk.defs \
	--override $(srcdir)/mooedit-pygtk.override \
	--outfilename mooedit-pygtk.c \
	$(srcdir)/mooedit-pygtk.defs > mooedit-pygtk.c

mooapp-pygtk.c: $(srcdir)/mooapp-pygtk.defs $(srcdir)/mooapp-pygtk.override
	pygtk-codegen-2.0 --prefix moo_app \
	--register $(PYGTK_DEFS_DIR)/gtk-types.defs \
	--register $(PYGTK_DEFS_DIR)/gdk-types.defs \
	--register $(srcdir)/mooedit-pygtk.defs \
	--register $(srcdir)/mooterm-pygtk.defs \
	--register $(srcdir)/mooutils-pygtk.defs \
	--override $(srcdir)/mooapp-pygtk.override \
	--outfilename mooapp-pygtk.c \
	$(srcdir)/mooapp-pygtk.defs > mooapp-pygtk.c
