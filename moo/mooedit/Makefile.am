SUBDIRS = plugins syntax

mooedit_toolsdir = $(MOO_DATA_DIR)
mooedit_tools_DATA = tools.cfg menu.cfg

mooedit_include_headers =              \
	moocmdview.h                   \
	moocompletion.h                \
	mooedit-actions.h              \
	mooedit-script.h               \
	mooedit.h                      \
	mooeditconfig.h                \
	mooeditor.h                    \
	mooeditprefs.h                 \
	mooeditwindow.h                \
	mooindenter.h                  \
	moolang.h                      \
	moolangmgr.h                   \
	moolinemark.h                  \
	moolineview.h                  \
	mooplugin.h                    \
	mooplugin-loader.h             \
	mooplugin-macro.h              \
	mootextbuffer.h                \
	mootextiter.h                  \
	mootextpopup.h                 \
	mootextsearch.h                \
	mootextstyle.h                 \
	mootextstylescheme.h           \
	mootextview.h                  \
	moousertools.h

mooedit_noinst_headers =               \
	gtksourceiter.h                \
	mooedit-private.h              \
	mooeditdialogs.h               \
	mooeditfileops.h               \
	mooeditfiltersettings.h        \
	mooeditprefs-glade.h           \
	mooeditprogress-glade.h        \
	mooeditsavemultiple-glade.h    \
	moofold.h                      \
	moohighlighter.h               \
	moolang-aux.h                  \
	moolang-parser.h               \
	moolang-private.h              \
	moolang-rules.h                \
	moolang-strings.h              \
	moolinebuffer.h                \
	moopluginprefs-glade.h         \
	mooprint-glade.h               \
	mootext-private.h              \
	mootextbox.h                   \
	mootextbtree.h                 \
	mootextfind-glade.h            \
	mootextfind.h                  \
	mootextgotoline-glade.h        \
	mootextsearch-private.h        \
	mootextview-private.h          \
	quicksearch-glade.h            \
	statusbar-glade.h

mooedit_sources =                      \
	$(mooedit_include_headers)     \
	$(mooedit_noinst_headers)      \
	gtksourceiter.c                \
	moocmdview.c                   \
	moocompletion.c                \
	mooedit-actions.c              \
	mooedit-script.c               \
	mooedit.c                      \
	mooeditconfig.c                \
	mooeditdialogs.c               \
	mooeditfileops.c               \
	mooeditfiltersettings.c        \
	mooeditinput.c                 \
	mooeditor.c                    \
	mooeditprefs.c                 \
	mooeditprefspage.c             \
	mooeditwindow.c                \
	moofold.c                      \
	moohighlighter.c               \
	mooindenter.c                  \
	moolang-parser.c               \
	moolang-rules.c                \
	moolang.c                      \
	moolangmgr.c                   \
	moolinebuffer.c                \
	moolinemark.c                  \
	moolineview.c                  \
	mooplugin.c                    \
	mooplugin-loader.c             \
	mootextbox.c                   \
	mootextbtree.c                 \
	mootextbuffer.c                \
	mootextfind.c                  \
	mootextpopup.c                 \
	mootextsearch.c                \
	mootextstyle.c                 \
	mootextstylescheme.c           \
	mootextview.c                  \
	moousertools.c

mooedit_printing_sources =             \
	mootextprint.c                 \
	mootextprint.h

BUILT_SOURCES =                        \
	quicksearch-glade.h            \
	statusbar-glade.h              \
	mootextgotoline-glade.h        \
	mootextfind-glade.h            \
	moopluginprefs-glade.h         \
	mooprint-glade.h               \
	mooeditprefs-glade.h           \
	mooeditprogress-glade.h        \
	mooeditsavemultiple-glade.h


XML2H = $(srcdir)/../mooutils/xml2h.sh
mootextgotoline-glade.h: $(srcdir)/glade/mootextgotoline.glade $(XML2H)
	$(SHELL) $(XML2H) MOO_TEXT_GOTO_LINE_GLADE_UI $(srcdir)/glade/mootextgotoline.glade \
		> mootextgotoline-glade.h
mootextfind-glade.h: $(srcdir)/glade/mootextfind.glade $(XML2H)
	$(SHELL) $(XML2H) MOO_TEXT_FIND_GLADE_UI $(srcdir)/glade/mootextfind.glade \
		> mootextfind-glade.h
mooeditprefs-glade.h: $(srcdir)/glade/mooeditprefs.glade $(XML2H)
	$(SHELL) $(XML2H) MOO_EDIT_PREFS_GLADE_UI $(srcdir)/glade/mooeditprefs.glade \
		> mooeditprefs-glade.h
mooeditprogress-glade.h: $(srcdir)/glade/mooeditprogress.glade $(XML2H)
	$(SHELL) $(XML2H) MOO_EDIT_PROGRESS_GLADE_XML $(srcdir)/glade/mooeditprogress.glade \
		> mooeditprogress-glade.h
moopluginprefs-glade.h: $(srcdir)/glade/moopluginprefs.glade $(XML2H)
	$(SHELL) $(XML2H) MOO_PLUGIN_PREFS_GLADE_UI $(srcdir)/glade/moopluginprefs.glade \
		> moopluginprefs-glade.h
mooeditsavemultiple-glade.h: $(srcdir)/glade/mooeditsavemult.glade $(XML2H)
	$(SHELL) $(XML2H) MOO_EDIT_SAVE_MULTIPLE_GLADE_UI $(srcdir)/glade/mooeditsavemult.glade \
		> mooeditsavemultiple-glade.h
quicksearch-glade.h: $(srcdir)/glade/quicksearch.glade $(XML2H)
	$(SHELL) $(XML2H) QUICK_SEARCH_GLADE_XML $(srcdir)/glade/quicksearch.glade \
		> quicksearch-glade.h
statusbar-glade.h: $(srcdir)/glade/statusbar.glade $(XML2H)
	$(SHELL) $(XML2H) STATUSBAR_GLADE_XML $(srcdir)/glade/statusbar.glade \
		> statusbar-glade.h
mooprint-glade.h: $(srcdir)/glade/mooprint.glade $(XML2H)
	$(SHELL) $(XML2H) MOO_PRINT_GLADE_XML $(srcdir)/glade/mooprint.glade \
		> mooprint-glade.h

EXTRA_DIST =				\
	glade/statusbar.glade		\
	glade/quicksearch.glade		\
	glade/mootextgotoline.glade	\
	glade/mootextfind.glade		\
	glade/mooeditprefs.glade	\
	glade/mooeditprogress.glade	\
	glade/mooeditsavemult.glade	\
	glade/moopluginprefs.glade	\
	glade/mooprint.glade		\
	$(mooedit_tools_DATA)

if GTK_2_10
mooedit_sources += $(mooedit_printing_sources)
else
EXTRA_DIST += $(mooedit_printing_sources)
endif

noinst_LTLIBRARIES = libmooedit.la
libmooedit_la_SOURCES = $(mooedit_sources)
libmooedit_la_LIBADD = plugins/libmooeditplugins.la

AM_CFLAGS =			\
	-I..			\
	-I$(top_builddir)	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)

if MOO_INSTALL_HEADERS
mooedit_includedir = $(moo_includedir)/mooedit
mooedit_include_HEADERS = $(mooedit_include_headers)
endif
