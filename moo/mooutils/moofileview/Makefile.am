SUBDIRS = glade

noinst_LTLIBRARIES = libmoofileview.la

pixmaps = symlink.png symlink-small.png
EXTRA_DIST = $(pixmaps)
BUILT_SOURCES = symlink.h moobookmarkmgr-glade.h

symlink.h: $(pixmaps)
	gdk-pixbuf-csource --static --build-list           \
	SYMLINK_ARROW $(srcdir)/symlink.png                \
	SYMLINK_ARROW_SMALL $(srcdir)/symlink-small.png    \
	> symlink.h

moobookmarkmgr-glade.h: $(srcdir)/glade/bookmark_editor.glade $(srcdir)/../xml2h.sh
	sh $(srcdir)/../xml2h.sh MOO_BOOKMARK_MGR_GLADE_UI \
	   $(srcdir)/glade/bookmark_editor.glade > moobookmarkmgr-glade.h

real_src_dir = `cd $(srcdir) && pwd`

AM_CFLAGS =                             \
    $(GTHREAD_CFLAGS)                   \
    -I$(top_builddir)                   \
    -I$(top_builddir)/$(MOO_SRC_PREFIX) \
    $(MOO_CFLAGS)                       \
    $(DEBUG_CFLAGS)                     \
    $(CFLAGS)

libmoofileview_la_SOURCES =     \
    moobookmarkmgr.c            \
    moobookmarkmgr.h            \
    moobookmarkmgr-glade.h      \
    moofile.c                   \
    moofile.h                   \
    moofileentry.c              \
    moofileentry.h              \
    moofilesystem.c             \
    moofilesystem.h             \
    moofileview.c               \
    moofileview.h               \
    moofileview-dialogs.c       \
    moofileview-dialogs.h       \
    moofileview-private.h       \
    moofoldermodel.c            \
    moofoldermodel.h            \
    moofoldermodel-private.h    \
    mooiconview.c               \
    mooiconview.h               \
    symlink.h
