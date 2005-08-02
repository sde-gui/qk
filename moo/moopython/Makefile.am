#
#   moopython/Makefile.am
#

DEFAULT_INCLUDES =
AM_CFLAGS =                                 \
    -I$(top_builddir)                       \
    -I$(top_builddir)/$(MOO_SRC_PREFIX)     \
    $(MOO_CFLAGS)                           \
    $(PYTHON_DEBUG_CFLAGS)                  \
    $(CFLAGS)                               \
    $(GTK_CFLAGS)                           \
    $(PYTHON_INCLUDES)                      \
    $(PYGTK_CFLAGS)                         \
    -DG_LOG_DOMAIN=\"Moo\"                  \
    -DMOO_VERSION=\"$(MOO_VERSION)\"        \
    -DMOO_VERSION_MAJOR=$(MOO_VERSION_MAJOR)\
    -DMOO_VERSION_MINOR=$(MOO_VERSION_MINOR)\
    -DMOO_VERSION_MICRO=$(MOO_VERSION_MICRO)


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
    mooui-mod.c             \
    mooui-pygtk.defs        \
    mooui-pygtk.override    \
    mooutils-mod.c          \
    mooutils-pygtk.defs     \
    mooutils-pygtk.override

nodist_libmoopython_la_SOURCES =
BUILT_SOURCES =
CLEANFILES =

if BUILD_MOOUTILS
libmoopython_la_SOURCES += mooutils-mod.c
nodist_libmoopython_la_SOURCES += mooutils-pygtk.c
BUILT_SOURCES += mooutils-pygtk.c
CLEANFILES += mooutils-pygtk.c
AM_CFLAGS += -DBUILD_MOOUTILS
endif BUILD_MOOUTILS

if BUILD_MOOUI
libmoopython_la_SOURCES += mooui-mod.c
nodist_libmoopython_la_SOURCES += mooui-pygtk.c
BUILT_SOURCES += mooui-pygtk.c
CLEANFILES += mooui-pygtk.c
AM_CFLAGS += -DBUILD_MOOUI
endif BUILD_MOOUI

if BUILD_MOOTERM
libmoopython_la_SOURCES += mooterm-mod.c
nodist_libmoopython_la_SOURCES += mooterm-pygtk.c
BUILT_SOURCES += mooterm-pygtk.c
CLEANFILES += mooterm-pygtk.c
AM_CFLAGS += -DBUILD_MOOTERM
endif BUILD_MOOTERM

if BUILD_MOOEDIT
libmoopython_la_SOURCES += mooedit-mod.c
nodist_libmoopython_la_SOURCES += mooedit-pygtk.c
BUILT_SOURCES += mooedit-pygtk.c
CLEANFILES += mooedit-pygtk.c
AM_CFLAGS += -DBUILD_MOOEDIT
endif BUILD_MOOEDIT

if BUILD_MOOAPP
libmoopython_la_SOURCES += mooapp-mod.c
nodist_libmoopython_la_SOURCES += mooapp-pygtk.c
BUILT_SOURCES += mooapp-pygtk.c
CLEANFILES += mooapp-pygtk.c
AM_CFLAGS += -DBUILD_MOOAPP
endif BUILD_MOOAPP

mooutils-pygtk.c: $(srcdir)/mooutils-pygtk.defs $(srcdir)/mooutils-pygtk.override
	pygtk-codegen-2.0 --prefix moo_utils \
	--register $(PYGTK_DEFS_DIR)/gtk-types.defs \
	--register $(PYGTK_DEFS_DIR)/gdk-types.defs \
	--override $(srcdir)/mooutils-pygtk.override \
	--outfilename mooutils-pygtk.c \
	$(srcdir)/mooutils-pygtk.defs > mooutils-pygtk.c

mooui-pygtk.c: $(srcdir)/mooui-pygtk.defs $(srcdir)/mooui-pygtk.override
	pygtk-codegen-2.0 --prefix moo_ui \
	--register $(PYGTK_DEFS_DIR)/gtk-types.defs \
	--register $(PYGTK_DEFS_DIR)/gdk-types.defs \
	--register $(srcdir)/mooutils-pygtk.defs \
	--override $(srcdir)/mooui-pygtk.override \
	--outfilename mooui-pygtk.c \
	$(srcdir)/mooui-pygtk.defs > mooui-pygtk.c

mooterm-pygtk.c: $(srcdir)/mooterm-pygtk.defs $(srcdir)/mooterm-pygtk.override
	pygtk-codegen-2.0 --prefix moo_term \
	--register $(PYGTK_DEFS_DIR)/gtk-types.defs \
	--register $(PYGTK_DEFS_DIR)/gdk-types.defs \
	--register $(srcdir)/mooui-pygtk.defs \
	--register $(srcdir)/mooutils-pygtk.defs \
	--override $(srcdir)/mooterm-pygtk.override \
	--outfilename mooterm-pygtk.c \
	$(srcdir)/mooterm-pygtk.defs > mooterm-pygtk.c

mooedit-pygtk.c: $(srcdir)/mooedit-pygtk.defs $(srcdir)/mooedit-pygtk.override
	pygtk-codegen-2.0 --prefix moo_edit \
	--register $(PYGTK_DEFS_DIR)/gtk-types.defs \
	--register $(PYGTK_DEFS_DIR)/gdk-types.defs \
	--register $(srcdir)/mooui-pygtk.defs \
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
	--register $(srcdir)/mooui-pygtk.defs \
	--register $(srcdir)/mooutils-pygtk.defs \
	--override $(srcdir)/mooapp-pygtk.override \
	--outfilename mooapp-pygtk.c \
	$(srcdir)/mooapp-pygtk.defs > mooapp-pygtk.c
