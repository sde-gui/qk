#
#   mooedit/gtksourceview/Makefile.am
#

AM_CFLAGS =\
    -I$(top_builddir)                       \
    -I$(builddir)                           \
    -I$(srcdir)                             \
    $(PYTHON_INCLUDES)                      \
    $(PYGTK_CFLAGS)                         \
    $(MOO_CFLAGS)                           \
    -I$(top_builddir)/$(MOO_SRC_PREFIX)     \
    $(CFLAGS)                               \
    -DGETTEXT_PACKAGE=\"gtksourceview-1.0\" \
    -DG_LOG_DOMAIN=\"GtkSourceView\"

noinst_LTLIBRARIES = libgtksourceview.la

libgtksourceview_la_SOURCES =   \
    gtksourcebuffer.c           \
    gtksourcebuffer.h           \
    gtksourceregex.c            \
    gtksourceregex.h            \
    gtksourcetag.c              \
    gtksourcetag.h              \
    gtksourcetag-private.h      \
    gtksourceview.c             \
    gtksourceview.h             \
    gtksourceiter.c             \
    gtksourceiter.h             \
    gtksourcemarker.c           \
    gtksourcemarker.h           \
    gtksourcetagstyle.h         \
    gtksourcetagtable.c         \
    gtksourcetagtable.h         \
    gtksourceundomanager.c      \
    gtksourceundomanager.h      \
    gtksourceview-i18n.c        \
    gtksourceview-i18n.h        \
    gtksourceview-marshal.c     \
    gtksourceview-marshal.h     \
    gtksourceview-typebuiltins.c\
    gtksourceview-typebuiltins.h\
    gtktextregion.c             \
    gtktextregion.h

EXTRA_DIST = \
    gtksourceview-marshal.list

# if USE_PYGTK
# libgtksourceview_la_SOURCES += gtksourceview-pygtk.c
# endif

# BUILT_SOURCES = gtksourceview-pygtk.c
# EXTRA_DIST += gtksourceview-pygtk.c gtksourceview-pygtk.defs gtksourceview-pygtk.override
# 
# gtksourceview-pygtk.c: $(srcdir)/gtksourceview-pygtk.defs $(srcdir)/gtksourceview-pygtk.override
# 	pygtk-codegen-2.0 --prefix moo_gtksourceview \
# 	--register $(PYGTK_DEFS_DIR)/gtk-types.defs \
# 	--register $(PYGTK_DEFS_DIR)/gdk-types.defs \
# 	--override $(srcdir)/gtksourceview-pygtk.override \
# 	$(srcdir)/gtksourceview-pygtk.defs > $(srcdir)/gtksourceview-pygtk.c
