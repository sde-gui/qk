#
#   mooutils/newgtk/Makefile.am
#

DEFAULT_INCLUDES =
AM_CFLAGS =                             \
    -I$(top_builddir)                   \
    -I$(top_srcdir)/$(MOO_SRC_PREFIX)   \
    -I$(top_builddir)/$(MOO_SRC_PREFIX) \
    $(M_CFLAGS)                         \
    $(CFLAGS)                           \
    $(XML_CFLAGS)                       \
    $(GTK_CFLAGS)                       \
    -DG_LOG_DOMAIN=\"Gtk\"

noinst_LTLIBRARIES = libnewgtk.la

libnewgtk_la_SOURCES =  \
    goption.c           \
    goption.h           \
    gtkcolorbutton.c    \
    gtkcolorbutton.h    \
    gtkfontbutton.c     \
    gtkfontbutton.h
