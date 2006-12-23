# XXX it needs plugins to be built first but plugins want mooedit-enums.h
SUBDIRS = gtksourceview plugins language-specs

mooedit_toolsdir = $(MOO_DATA_DIR)
mooedit_tools_DATA = context.cfg menu.cfg filters.cfg

dummy_targets =				\
	moocommand-exe.o		\
	moocommand-exe-private.o	\
	moolangmgr-private.o
CLEANFILES = $(dummy_targets)
moocommand-exe.o: moocommand-exe-unix.o ; echo stamp > $(@F)
moocommand-exe-private.o: moocommand-exe-unix.o ; echo stamp > $(@F)
moolangmgr-private.o: moolangmgr.o ; echo stamp > $(@F)

unix_sources =				\
	moocommand-exe-unix.c

mooedit_include_headers =		\
	moocmdview.h			\
	mooedit-bookmarks.h		\
	mooedit-enums.h			\
	mooedit-script.h		\
	mooedit.h			\
	mooeditconfig.h			\
	mooeditor.h			\
	mooeditprefs.h			\
	mooeditwindow.h			\
	mooindenter.h			\
	moolinemark.h			\
	moolineview.h			\
	moooutputfilter.h		\
	mooplugin-loader.h		\
	mooplugin-macro.h		\
	mooplugin.h			\
	mootextbuffer.h			\
	mootextiter.h			\
	mootextsearch.h			\
	mootextstyle.h			\
	mootextstylescheme.h		\
	mootextview.h

mooedit_noinst_headers =		\
	medit-ui.h			\
	moocommand-exe.h		\
	moocommand-script.h		\
	moocommand-private.h		\
	moocommand-exe-private.h	\
	moocommand.h			\
	moocommanddisplay.h		\
	mooedit-private.h		\
	mooedit-ui.h			\
	mooeditaction-factory.h		\
	mooeditaction.h			\
	mooeditdialogs.h		\
	mooeditfileops.h		\
	mooeditfiltersettings.h		\
	mooeditor-private.h		\
	mooeditprefs-glade.h		\
	mooeditprogress-glade.h		\
	mooeditsavemultiple-glade.h	\
	mooedittools-glade.h		\
	moofold.h			\
	mookeyfile.h			\
	moolang-private.h		\
	moolang.h			\
	moolangmgr-private.h		\
	moolangmgr.h			\
	moolinebuffer.h			\
	moooutputfiltersimple.h		\
	moopluginprefs-glade.h		\
	mooprint-glade.h		\
	mootext-private.h		\
	mootextbox.h			\
	mootextbtree.h			\
	mootextfind-glade.h		\
	mootextfind.h			\
	mootextgotoline-glade.h		\
	mootextsearch-private.h		\
	mootextview-private.h		\
	moousertools-prefs.h		\
	moousertools.h			\
	quicksearch-glade.h		\
	statusbar-glade.h

mooedit_sources =			\
	$(mooedit_include_headers)	\
	$(mooedit_noinst_headers)	\
	$(moocommand_stuff)		\
	moocmdview.c			\
	moocommand-script.c		\
	moocommand.c			\
	moocommanddisplay.c		\
	mooedit-bookmarks.c		\
	mooedit-enums.c			\
	mooedit-script.c		\
	mooedit.c			\
	mooeditaction-factory.c		\
	mooeditaction.c			\
	mooeditconfig.c			\
	mooeditdialogs.c		\
	mooeditfileops.c		\
	mooeditfiltersettings.c		\
	mooeditinput.c			\
	mooeditor.c			\
	mooeditprefs.c			\
	mooeditprefspage.c		\
	mooeditwindow.c			\
	moofold.c			\
	mooindenter.c			\
	mookeyfile.c			\
	moolang.c			\
	moolangmgr.c			\
	moolinebuffer.c			\
	moolinemark.c			\
	moolineview.c			\
	moooutputfilter.c		\
	moooutputfiltersimple.c		\
	mooplugin-loader.c		\
	mooplugin.c			\
	mootextbox.c			\
	mootextbtree.c			\
	mootextbuffer.c			\
	mootextfind.c			\
	mootextsearch.c			\
	mootextstylescheme.c		\
	mootextview.c			\
	moousertools-prefs.c		\
	moousertools.c

mooedit_printing_sources =		\
	mooprintpreview.h		\
	mooprintpreview-glade.h		\
	mooprintpreview.c		\
	mootextprint.c			\
	mootextprint-private.h		\
	mootextprint.h

BUILT_SOURCES =				\
	quicksearch-glade.h		\
	statusbar-glade.h		\
	medit-ui.h			\
	mootextgotoline-glade.h		\
	mootextfind-glade.h		\
	moopluginprefs-glade.h		\
	mooprint-glade.h		\
	mooeditprefs-glade.h		\
	mooeditprogress-glade.h		\
	mooeditsavemultiple-glade.h	\
	mooedittools-glade.h		\
	mooedit-enums.c			\
	mooedit-enums.h			\
	mooedit-ui.h			\
	mooprintpreview-glade.h

mooedit_enums_headers =			\
	moocommand.h			\
	mooeditconfig.h			\
	mooeditwindow.h			\
	mooedit.h			\
	mootextfind.h			\
	mootextsearch.h			\
	mootextstyle.h			\
	mootextview.h

CLEANFILES += stamp-mooedit-enums.h stamp-mooedit-enums.c
DISTCLEANFILES = mooedit-enums.h mooedit-enums.c
mooedit-enums.h: stamp-mooedit-enums.h
	@true
mooedit-enums.c: stamp-mooedit-enums.c
	@true
stamp-mooedit-enums.h: $(mooedit_enums_headers) Makefile
	( cd $(srcdir) && glib-mkenums \
			--fhead "#ifndef __MOO_EDIT_ENUMS_H__\n#define __MOO_EDIT_ENUMS_H__\n\n#include <glib-object.h>\n" \
			--fhead "\nG_BEGIN_DECLS\n\n\n" \
			--fprod "/* enumerations from \"mooedit/@filename@\" */\n" \
			--vhead "GType @enum_name@_get_type (void) G_GNUC_CONST;\n#define MOO_TYPE_@ENUMSHORT@ (@enum_name@_get_type())\n\n" \
			--ftail "\nG_END_DECLS\n\n#endif /* __MOO_EDIT_ENUMS_H__ */" \
		$(mooedit_enums_headers) ) > xgen-gtbh \
	&& (cmp -s xgen-gtbh mooedit-enums.h || cp xgen-gtbh mooedit-enums.h ) \
	&& rm -f xgen-gtbh \
	&& echo timestamp > $(@F)
stamp-mooedit-enums.c: $(mooedit_enums_headers) Makefile
	( cd $(srcdir) && glib-mkenums \
			--fhead "#include \"mooedit/mooedit-enums.h\"\n\n" \
			--fprod "#include \"mooedit/@filename@\"\n\n" \
			--vhead "GType\n@enum_name@_get_type (void)\n{\n    static GType etype = 0;\n    if (etype == 0)\n    {\n        static const G@Type@Value values[] = {" \
			--vprod "            { @VALUENAME@, (char*) \"@VALUENAME@\", (char*) \"@valuenick@\" }," \
			--vtail "            { 0, NULL, NULL }\n        };\n        etype = g_@type@_register_static (\"@EnumName@\", values);\n    }\n    return etype;\n}\n\n" \
		$(mooedit_enums_headers) ) > xgen-gtbc \
	&& (cmp -s xgen-gtbc mooedit-enums.c || cp xgen-gtbc mooedit-enums.c ) \
	&& rm -f xgen-gtbc \
	&& echo timestamp > $(@F)


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
mooedittools-glade.h: $(srcdir)/glade/mooedittools.glade $(XML2H)
	$(SHELL) $(XML2H) MOO_EDIT_TOOLS_GLADE_XML $(srcdir)/glade/mooedittools.glade \
		> mooedittools-glade.h
mooprintpreview-glade.h: $(srcdir)/glade/mooprintpreview.glade $(XML2H)
	$(SHELL) $(XML2H) MOO_PRINT_PREVIEW_GLADE_XML $(srcdir)/glade/mooprintpreview.glade \
		> mooprintpreview-glade.h

mooedit-ui.h: mooedit-ui.xml $(XML2H)
	$(SHELL) $(XML2H) MOO_EDIT_UI_XML $(srcdir)/mooedit-ui.xml > mooedit-ui.h.tmp && \
	mv mooedit-ui.h.tmp mooedit-ui.h
medit-ui.h: medit-ui.xml $(XML2H)
	$(SHELL) $(XML2H) MEDIT_UI_XML $(srcdir)/medit-ui.xml > medit-ui.h.tmp && \
	mv medit-ui.h.tmp medit-ui.h

EXTRA_DIST =				\
	medit-ui.xml			\
	mooedit-ui.xml			\
	glade/statusbar.glade		\
	glade/quicksearch.glade		\
	glade/mootextgotoline.glade	\
	glade/mootextfind.glade		\
	glade/mooeditprefs.glade	\
	glade/mooeditprogress.glade	\
	glade/mooeditsavemult.glade	\
	glade/mooedittools.glade	\
	glade/moopluginprefs.glade	\
	glade/mooprint.glade		\
	glade/mooprintpreview.glade	\
	$(mooedit_tools_DATA)

if GTK_2_10
mooedit_sources += $(mooedit_printing_sources)
else
EXTRA_DIST += $(mooedit_printing_sources)
endif

if MOO_OS_UNIX
mooedit_sources += $(unix_sources)
else
EXTRA_DIST += $(unix_sources)
endif

noinst_LTLIBRARIES = libmooedit.la
libmooedit_la_SOURCES = $(mooedit_sources)
libmooedit_la_LIBADD = plugins/libmooeditplugins.la gtksourceview/libgtksourceview.la

AM_CFLAGS =			\
	-I..			\
	-I$(top_builddir)	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)

if MOO_INSTALL_HEADERS
mooedit_includedir = $(moo_includedir)/mooedit
mooedit_include_HEADERS = $(mooedit_include_headers)
endif
