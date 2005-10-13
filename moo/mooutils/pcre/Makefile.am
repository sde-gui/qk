noinst_LTLIBRARIES = libpcre.la

AM_CFLAGS =                             \
    -I$(top_builddir)                   \
    -I$(top_builddir)/$(MOO_SRC_PREFIX) \
    -I$(srcdir)                         \
    $(PYTHON_INCLUDES)                  \
    $(PYGTK_CFLAGS)                     \
    $(MOO_CFLAGS)                       \
    $(CFLAGS)

libpcre_la_SOURCES =    \
    get.c               \
    internal.h          \
    pcre-config.h       \
    maketables.c        \
    chartables.c        \
    pcre.c              \
    pcre.h              \
    study.c
