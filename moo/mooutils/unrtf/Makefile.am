#
#   mooutils/unrtf/Makefile.am
#

DEFAULT_INCLUDES =
AM_CFLAGS =                             \
    -I$(top_builddir)                   \
    -I$(top_srcdir)/$(MOO_SRC_PREFIX)   \
    -I$(top_builddir)/$(MOO_SRC_PREFIX) \
    $(GLIB_CFLAGS)                      \
    $(M_CFLAGS)                         \
    $(CFLAGS)

libunrtf_la_src =       \
    attr.c              \
    attr.h              \
    convert.c           \
    convert.h           \
    defs.h              \
    entity.c            \
    hash.c              \
    hash.h              \
    output.c            \
    output.h            \
    parse.c             \
    parse.h             \
    util.c              \
    util.h              \
    word.c              \
    word.h

EXTRA_DIST = README $(libunrtf_la_src) html.c html.h

# noinst_LTLIBRARIES = libunrtf.la
# libunrtf_la_SOURCES = $(libunrtf_la_src)
