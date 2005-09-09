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
    gtksourceview-typebuiltins.c\
    gtksourceview-typebuiltins.h\
    gtktextregion.c             \
    gtktextregion.h
