MOO = ../../moo
MOO_W = ..\..\moo
XML2H = $(MOO)/mooutils/xml2h.py

mooutils = $(MOO)/mooutils
mooutils_srcdir = $(MOO)/mooutils
mooedit = $(MOO)/mooedit
mooedit_srcdir = $(MOO)/mooedit
tests = ../../tests
tests_w = ..\..\tests
moofileview = $(mooutils)/moofileview
moofileview_srcdir = $(moofileview)
mooedit_plugins = $(mooedit)/plugins
mooedit_plugins_srcdir = $(mooedit_plugins)
astrings = $(mooedit_plugins)/activestrings
astrings_srcdir = $(astrings)
fileselector = $(mooedit_plugins)/fileselector
fileselector_srcdir = $(fileselector)

GENERATED =                                 \
    config.h                                \
    $(mooutils)/moomarshals.h               \
    $(mooutils)/moomarshals.c               \
    $(tests)/medit-ui.h                     \
    $(MOO)/mooapp/mooappabout-glade.h       \
    $(mooutils)/mooaccelbutton-glade.h      \
    $(mooutils)/mooaccelprefs-glade.h       \
    $(mooutils)/moologwindow-glade.h        \
    $(mooedit)/quicksearch-glade.h          \
    $(mooedit)/statusbar-glade.h            \
    $(mooedit)/mootextgotoline-glade.h      \
    $(mooedit)/mootextfind-glade.h          \
    $(mooedit)/moopluginprefs-glade.h       \
    $(mooedit)/mooprint-glade.h             \
    $(mooedit)/mooeditprefs-glade.h         \
    $(mooedit)/mooeditsavemultiple-glade.h  \
    $(mooutils)/stock-moo.h                 \
    $(moofileview)/moofileview-ui.h         \
    $(moofileview)/moofileprops-glade.h     \
    $(moofileview)/moocreatefolder-glade.h  \
    $(moofileview)/moofileviewdrop-glade.h  \
    $(moofileview)/symlink.h                \
    $(moofileview)/moobookmarkmgr-glade.h   \
    $(astrings)/as-plugin-glade.h           \
	$(fileselector)/moofileselector-glade.h	\
	$(fileselector)/moofileselector-prefs-glade.h

all-before: $(GENERATED)

LINKOBJ = *.o $(RES)

##############################################################
# Marshalers
#
$(MOO)/mooutils/moomarshals.h: $(MOO)/mooutils/moomarshals.list
	glib-genmarshal --prefix=_moo_marshal --header $(MOO)/mooutils/moomarshals.list > $(MOO)/mooutils/moomarshals.h

$(MOO)/mooutils/moomarshals.c: $(MOO)/mooutils/moomarshals.list
	glib-genmarshal --prefix=_moo_marshal --body $(MOO)/mooutils/moomarshals.list > $(MOO)/mooutils/moomarshals.c


##############################################################
# config.h
#
config.h: ../../config.h.win32
	copy /Y ..\..\config.h.win32 config.h


##############################################################
# glade files
#
$(mooutils)/mooaccelbutton-glade.h: $(mooutils_srcdir)/glade/shortcutdialog.glade $(XML2H)
	python $(XML2H) MOO_ACCEL_BUTTON_GLADE_UI \
	   $(mooutils_srcdir)/glade/shortcutdialog.glade > $(mooutils)/mooaccelbutton-glade.h
$(mooutils)/mooaccelprefs-glade.h: $(mooutils_srcdir)/glade/shortcutsprefs.glade $(XML2H)
	python $(XML2H) MOO_SHORTCUTS_PREFS_GLADE_UI \
	   $(mooutils_srcdir)/glade/shortcutsprefs.glade > $(mooutils)/mooaccelprefs-glade.h
$(mooutils)/moologwindow-glade.h: $(mooutils_srcdir)/glade/moologwindow.glade $(XML2H)
	python $(XML2H) MOO_LOG_WINDOW_GLADE_UI \
	   $(mooutils_srcdir)/glade/moologwindow.glade > $(mooutils)/moologwindow-glade.h
$(MOO)/mooapp/mooappabout-glade.h: $(MOO)/mooapp/glade/mooappabout.glade $(XML2H)
	python $(XML2H) MOO_APP_ABOUT_GLADE_UI \
	   $(MOO)/mooapp/glade/mooappabout.glade > $(MOO)/mooapp/mooappabout-glade.h
$(mooedit)/mootextgotoline-glade.h: $(mooedit_srcdir)/glade/mootextgotoline.glade $(XML2H)
	python $(XML2H) MOO_TEXT_GOTO_LINE_GLADE_UI $(mooedit_srcdir)/glade/mootextgotoline.glade \
		> $(mooedit)/mootextgotoline-glade.h
$(mooedit)/mootextfind-glade.h: $(mooedit_srcdir)/glade/mootextfind.glade $(XML2H)
	python $(XML2H) MOO_TEXT_FIND_GLADE_UI $(mooedit_srcdir)/glade/mootextfind.glade \
		> $(mooedit)/mootextfind-glade.h
$(mooedit)/mooeditprefs-glade.h: $(mooedit_srcdir)/glade/mooeditprefs.glade $(XML2H)
	python $(XML2H) MOO_EDIT_PREFS_GLADE_UI $(mooedit_srcdir)/glade/mooeditprefs.glade \
		> $(mooedit)/mooeditprefs-glade.h
$(mooedit)/moopluginprefs-glade.h: $(mooedit_srcdir)/glade/moopluginprefs.glade $(XML2H)
	python $(XML2H) MOO_PLUGIN_PREFS_GLADE_UI $(mooedit_srcdir)/glade/moopluginprefs.glade \
		> $(mooedit)/moopluginprefs-glade.h
$(mooedit)/mooeditsavemultiple-glade.h: $(mooedit_srcdir)/glade/mooeditsavemult.glade $(XML2H)
	python $(XML2H) MOO_EDIT_SAVE_MULTIPLE_GLADE_UI $(mooedit_srcdir)/glade/mooeditsavemult.glade \
		> $(mooedit)/mooeditsavemultiple-glade.h
$(mooedit)/quicksearch-glade.h: $(mooedit_srcdir)/glade/quicksearch.glade $(XML2H)
	python $(XML2H) QUICK_SEARCH_GLADE_XML $(mooedit_srcdir)/glade/quicksearch.glade \
		> $(mooedit)/quicksearch-glade.h
$(mooedit)/statusbar-glade.h: $(mooedit_srcdir)/glade/statusbar.glade $(XML2H)
	python $(XML2H) STATUSBAR_GLADE_XML $(mooedit_srcdir)/glade/statusbar.glade \
		> $(mooedit)/statusbar-glade.h
$(mooedit)/mooprint-glade.h: $(mooedit_srcdir)/glade/mooprint.glade $(XML2H)
	python $(XML2H) MOO_PRINT_GLADE_XML $(mooedit_srcdir)/glade/mooprint.glade \
		> $(mooedit)/mooprint-glade.h
$(moofileview)/moofileview-ui.h: $(moofileview_srcdir)/moofileview-ui.xml $(XML2H)
	python $(XML2H) MOO_FILE_VIEW_UI                                       \
	   $(moofileview_srcdir)/moofileview-ui.xml >                      \
		$(moofileview)/moofileview-ui.h
$(moofileview)/moofileprops-glade.h: $(moofileview_srcdir)/glade/moofileprops.glade $(XML2H)
	python $(XML2H) MOO_FILE_PROPS_GLADE_UI                                \
	   $(moofileview_srcdir)/glade/moofileprops.glade >                \
		$(moofileview)/moofileprops-glade.h
$(moofileview)/moocreatefolder-glade.h: $(moofileview_srcdir)/glade/moocreatefolder.glade $(XML2H)
	python $(XML2H) MOO_CREATE_FOLDER_GLADE_UI                             \
	   $(moofileview_srcdir)/glade/moocreatefolder.glade >             \
		$(moofileview)/moocreatefolder-glade.h
$(moofileview)/moobookmarkmgr-glade.h: $(moofileview_srcdir)/glade/bookmark_editor.glade $(XML2H)
	python $(XML2H) MOO_BOOKMARK_MGR_GLADE_UI                              \
	   $(moofileview_srcdir)/glade/bookmark_editor.glade >             \
		$(moofileview)/moobookmarkmgr-glade.h
$(moofileview)/moofileviewdrop-glade.h: $(moofileview_srcdir)/glade/drop.glade $(XML2H)
	python $(XML2H) MOO_FILE_VIEW_DROP_GLADE_UI                            \
	   $(moofileview_srcdir)/glade/drop.glade >                        \
		$(moofileview)/moofileviewdrop-glade.h
$(astrings)/as-plugin-glade.h: $(astrings)/as-plugin.glade $(XML2H)
	python $(XML2H) AS_PLUGIN_GLADE_UI $(astrings_srcdir)/as-plugin.glade > \
	   $(astrings)/as-plugin-glade.h
$(fileselector)/moofileselector-glade.h: $(fileselector)/moofileselector.glade $(XML2H)
	python $(XML2H) MOO_FILE_SELECTOR_GLADE_XML $(fileselector_srcdir)/moofileselector.glade > \
	   $(fileselector)/moofileselector-glade.h
$(fileselector)/moofileselector-prefs-glade.h: $(fileselector)/moofileselector-prefs.glade $(XML2H)
	python $(XML2H) MOO_FILE_SELECTOR_PREFS_GLADE_XML $(fileselector_srcdir)/moofileselector-prefs.glade > \
	   $(fileselector)/moofileselector-prefs-glade.h


moofileview_pixmaps =                          \
    $(moofileview)/symlink.png                 \
    $(moofileview)/symlink-small.png

$(moofileview)/symlink.h: $(moofileview_pixmaps)
	gdk-pixbuf-csource --static --build-list                           \
	SYMLINK_ARROW $(moofileview_srcdir)/symlink.png                    \
	SYMLINK_ARROW_SMALL $(moofileview_srcdir)/symlink-small.png        \
	> $(moofileview)/symlink.h


mooutils_pixmaps =                             \
    $(mooutils_srcdir)/pixmaps/gap.png         \
    $(mooutils_srcdir)/pixmaps/ggap.png        \
    $(mooutils_srcdir)/pixmaps/medit.png       \
    $(mooutils_srcdir)/pixmaps/close.png       \
    $(mooutils_srcdir)/pixmaps/detach.png      \
    $(mooutils_srcdir)/pixmaps/attach.png      \
    $(mooutils_srcdir)/pixmaps/keepontop.png   \
    $(mooutils_srcdir)/pixmaps/sticky.png

$(mooutils)/stock-moo.h: $(mooutils_pixmaps)
	gdk-pixbuf-csource --static --build-list                               \
		GAP_ICON  $(mooutils_srcdir)/pixmaps/gap.png                   \
		GGAP_ICON $(mooutils_srcdir)/pixmaps/ggap.png                  \
		MEDIT_ICON $(mooutils_srcdir)/pixmaps/medit.png                \
		MOO_CLOSE_ICON $(mooutils_srcdir)/pixmaps/close.png            \
		MOO_STICKY_ICON $(mooutils_srcdir)/pixmaps/sticky.png          \
		MOO_DETACH_ICON $(mooutils_srcdir)/pixmaps/detach.png          \
		MOO_ATTACH_ICON $(mooutils_srcdir)/pixmaps/attach.png          \
		MOO_KEEP_ON_TOP_ICON $(mooutils_srcdir)/pixmaps/keepontop.png  \
			> $(mooutils)/stock-moo.h


$(tests)/medit-ui.h: $(tests)/medit-ui.xml $(XML2H)
	python $(XML2H) MEDIT_UI $(tests)/medit-ui.xml > $(tests)/medit-ui.h
$(tests)/medit-app.c: $(tests)/medit-app.opag
	opag -f _medit_parse_options -O _medit_opt_ -A _medit_arg_ $(tests_w)\medit-app.opag $(tests_w)\medit-app.c
