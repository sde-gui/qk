#
#   mooutils/newgtk/Makefile.am
#

DEFAULT_INCLUDES =
AM_CFLAGS =                             \
    -I$(top_builddir)                   \
    -I$(top_srcdir)/$(MOO_SRC_PREFIX)   \
    -I$(top_builddir)/$(MOO_SRC_PREFIX) \
    $(MOO_DEBUG_CFLAGS)                 \
    $(CFLAGS)                           \
    $(XML_CFLAGS)                       \
    $(GTK_CFLAGS)                       \
    -DG_LOG_DOMAIN=\"Gtk\"

noinst_LTLIBRARIES = libnewgtk.la
libnewgtk_la_SOURCES = newgtk-dummy.c

glib_2_8_sources =      \
    gmappedfile.c       \
    gmappedfile.h

gtk_2_4_sources =       \
    gtkcolorbutton.c    \
    gtkcolorbutton.h    \
    gtkfontbutton.c     \
    gtkfontbutton.h

EXTRA_DIST =            \
    $(glib_2_8_sources) \
    $(gtk_2_4_sources)

if !GLIB_2_8
libnewgtk_la_SOURCES += $(glib_2_8_sources)
endif !GLIB_2_8

if !GTK_2_4
libnewgtk_la_SOURCES += $(gtk_2_4_sources)
endif
