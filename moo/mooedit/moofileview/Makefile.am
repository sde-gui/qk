SUBDIRS = glade

noinst_LTLIBRARIES = libmoofileview.la

EXTRA_DIST = symlink.png
BUILT_SOURCES = symlink.h

win_sources = moohandlewatch.h moohandlewatch.c
EXTRA_DIST += $(win_sources)

symlink.h: symlink.png symlink-small.png
	gdk-pixbuf-csource --static --build-list           \
	SYMLINK_ARROW $(srcdir)/symlink.png                \
	SYMLINK_ARROW_SMALL $(srcdir)/symlink-small.png    \
	> symlink.h

real_src_dir = `cd $(srcdir) && pwd`

AM_CFLAGS =                             \
    -DMOO_FILE_PROPS_GLADE_FILE=\"$(real_src_dir)/glade/fileprops.glade\" \
    -DMOO_CREATE_FOLDER_GLADE_FILE=\"$(real_src_dir)/glade/create_folder.glade\" \
    -DMOO_BOOKMARK_EDITOR_GLADE_FILE=\"$(real_src_dir)/glade/bookmark_editor.glade\" \
    $(GTHREAD_CFLAGS)                   \
    $(GLADE_CFLAGS)                     \
    -DXDG_PREFIX=_moo_edit_xdg          \
    -I$(top_builddir)                   \
    -I$(top_builddir)/$(MOO_SRC_PREFIX) \
    $(MOO_CFLAGS)                       \
    $(DEBUG_CFLAGS)                     \
    $(CFLAGS)                           \
    -DG_LOG_DOMAIN=\"Moo\"              \
    -D__MOO__                           \
    -DMOO_MARSHALS_H=\"mooutils/moomarshals.h\"

libmoofileview_la_SOURCES =     \
    moobookmarkmgr.c            \
    moobookmarkmgr.h            \
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
    moofilewatch.c              \
    moofilewatch.h              \
    moofiltermgr.c              \
    moofiltermgr.h              \
    moofoldermodel.c            \
    moofoldermodel.h            \
    moofoldermodel-private.h    \
    mooiconview.c               \
    mooiconview.h               \
    moosignal.h                 \
    symlink.h

if MINGW_BUILD
libmoofileview_la_SOURCES += $(win_sources)
# else
# libmoofileview_la_SOURCES += $(win_sources)
endif
