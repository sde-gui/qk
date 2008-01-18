SUBDIRS = gtksourceview plugins language-specs .

mooedit_toolsdir = $(MOO_DATA_DIR)
mooedit_tools_DATA = context.cfg menu.cfg filters.xml
mooedit_luadir = $(MOO_DATA_DIR)/lua
mooedit_lua_DATA = medit.lua

moolangmgr-private.lo: moolangmgr.lo
mootextview-private.lo: mootextview.lo
mootextsearch-private.lo: mootextsearch.lo
mooedit-private.lo: mooedit.lo

lua_sources =				\
	moocommand-lua.c		\
	moocommand-lua.h		\
	mooedit-lua.c			\
	mooedit-lua.h

mooedit_include_headers =		\
	moocmdview.h			\
	mooedit-bookmarks.h		\
	mooedit.h			\
	mooeditconfig.h			\
	mooedit-enums.h			\
	mooeditor.h			\
	mooeditprefs.h			\
	mooeditwindow.h			\
	mooindenter.h			\
	moolang.h			\
	moolangmgr.h			\
	moolinemark.h			\
	moolineview.h			\
	moooutputfilter.h		\
	mooplugin-loader.h		\
	mooplugin-macro.h		\
	mooplugin.h			\
	mootextbuffer.h			\
	mootextiter.h			\
	mootextsearch.h			\
	mootextstylescheme.h		\
	mootextview.h

completion_sources =			\
	moocompletionsimple.c		\
	moocompletionsimple.h		\
	mootextcompletion.c		\
	mootextcompletion.h		\
	mootextpopup.c			\
	mootextpopup.h

mooedit_sources =			\
	$(mooedit_include_headers)	\
	$(moocommand_stuff)		\
	$(completion_sources)		\
	moocmdview.c			\
	moocommand.c			\
	moocommanddisplay.c		\
	moocommanddisplay.h		\
	moocommand-exe.c		\
	moocommand-exe.h		\
	moocommand.h			\
	moocommand-private.h		\
	mooeditaction.c			\
	mooeditaction-factory.c		\
	mooeditaction-factory.h		\
	mooeditaction.h			\
	mooedit-bookmarks.c		\
	mooedit.c			\
	mooeditconfig.c			\
	mooeditdialogs.c		\
	mooeditdialogs.h		\
	mooedit-enums.c			\
	mooeditfileops.c		\
	mooeditfileops.h		\
	mooeditfiltersettings.c		\
	mooeditfiltersettings.h		\
	mooeditinput.c			\
	mooeditor.c			\
	mooeditor-private.h		\
	mooeditprefs.c			\
	mooeditprefspage.c		\
	mooedit-private.h		\
	mooeditwindow.c			\
	moofold.c			\
	moofold.h			\
	mooindenter.c			\
	mookeyfile.c			\
	mookeyfile.h			\
	moolang.c			\
	moolangmgr.c			\
	moolangmgr-private.h		\
	moolang-private.h		\
	moolinebuffer.c			\
	moolinebuffer.h			\
	moolinemark.c			\
	moolineview.c			\
	moooutputfilter.c		\
	moooutputfilterregex.c		\
	moooutputfilterregex.h		\
	mooplugin.c			\
	mooplugin-loader.c		\
	mootext-private.h		\
	mootextbox.c			\
	mootextbox.h			\
	mootextbtree.c			\
	mootextbtree.h			\
	mootextbuffer.c			\
	mootextfind.c			\
	mootextfind.h			\
	mootextsearch.c			\
	mootextsearch-private.h		\
	mootextstylescheme.c		\
	mootextview.c			\
	mootextview-private.h		\
	moousertools.c			\
	moousertools.h			\
	moousertools-prefs.c		\
	moousertools-prefs.h

mooedit_printing_sources =		\
	mooprintpreview.h		\
	mooprintpreview.c		\
	mootextprint.c			\
	mootextprint-private.h		\
	mootextprint.h

mooedit_enums_headers =			\
	moocommand.h			\
	mooeditconfig.h			\
	mooeditwindow.h			\
	mooedit.h			\
	mootextfind.h			\
	mootextsearch.h			\
	mootextview.h

BUILT_SOURCES = stamp-mooedit-enums-h stamp-mooedit-enums-c
CLEANFILES = stamp-mooedit-enums-h stamp-mooedit-enums-c
stamp-mooedit-enums-h: $(mooedit_enums_headers) Makefile
	( cd $(srcdir) && glib-mkenums \
			--fhead "#ifndef MOO_EDIT_ENUMS_H\n#define MOO_EDIT_ENUMS_H\n\n#include <glib-object.h>\n" \
			--fhead "\nG_BEGIN_DECLS\n\n\n" \
			--fprod "/* enumerations from \"mooedit/@filename@\" */\n" \
			--vhead "GType @enum_name@_get_type (void) G_GNUC_CONST;\n#define MOO_TYPE_@ENUMSHORT@ (@enum_name@_get_type())\n\n" \
			--ftail "\nG_END_DECLS\n\n#endif /* MOO_EDIT_ENUMS_H */" \
		$(mooedit_enums_headers) ) > xgen-gtbh \
	&& (cmp -s xgen-gtbh $(srcdir)/mooedit-enums.h || cp xgen-gtbh $(srcdir)/mooedit-enums.h ) \
	&& rm -f xgen-gtbh \
	&& echo timestamp > $(@F)
stamp-mooedit-enums-c: $(mooedit_enums_headers) Makefile
	( cd $(srcdir) && glib-mkenums \
			--fhead "#include \"mooedit/mooedit-enums.h\"\n\n" \
			--fprod "#include \"mooedit/@filename@\"\n\n" \
			--vhead "GType\n@enum_name@_get_type (void)\n{\n    static GType etype;\n    if (G_UNLIKELY (!etype))\n    {\n        static const G@Type@Value values[] = {" \
			--vprod "            { @VALUENAME@, (char*) \"@VALUENAME@\", (char*) \"@valuenick@\" }," \
			--vtail "            { 0, NULL, NULL }\n        };\n        etype = g_@type@_register_static (\"@EnumName@\", values);\n    }\n    return etype;\n}\n\n" \
		$(mooedit_enums_headers) ) > xgen-gtbc \
	&& (cmp -s xgen-gtbc $(srcdir)/mooedit-enums.c || cp xgen-gtbc $(srcdir)/mooedit-enums.c ) \
	&& rm -f xgen-gtbc \
	&& echo timestamp > $(@F)

@MOO_GLADE_SUBDIR_RULE@
glade_files =				\
	glade/mooeditprefs-general.glade\
	glade/mooeditprefs-view.glade	\
	glade/mooeditprefs-file.glade	\
	glade/mooeditprefs-langs.glade	\
	glade/mooeditprogress.glade	\
	glade/mooeditsavemult.glade	\
	glade/mooedittools.glade	\
	glade/moopluginprefs.glade	\
	glade/mooprint.glade		\
	glade/mootextfind.glade		\
	glade/mootextgotoline.glade	\
	glade/mooquicksearch.glade	\
	glade/moostatusbar.glade	\
	glade/mooprintpreview.glade
glade_sources = $(patsubst glade/%.glade,%-glade.h,$(glade_files))
nodist_mooedit_sources = $(glade_sources)
BUILT_SOURCES += $(glade_sources)
CLEANFILES += $(glade_sources)
EXTRA_DIST = $(glade_files)

XML2H = $(srcdir)/../mooutils/xml2h.sh
mooedit-ui.h: mooedit-ui.xml $(XML2H)
	$(SHELL) $(XML2H) mooedit_ui_xml $(srcdir)/mooedit-ui.xml > $@.tmp && \
	mv $@.tmp $@
medit-ui.h: medit-ui.xml $(XML2H)
	$(SHELL) $(XML2H) medit_ui_xml $(srcdir)/medit-ui.xml > $@.tmp && \
	mv $@.tmp $@
nodist_mooedit_sources += mooedit-ui.h medit-ui.h
BUILT_SOURCES += mooedit-ui.h medit-ui.h
CLEANFILES += mooedit-ui.h medit-ui.h

EXTRA_DIST +=				\
	$(mooedit_tools_DATA)		\
	$(mooedit_lua_DATA)		\
	medit-ui.xml			\
	mooedit-ui.xml

if GTK_2_10
mooedit_sources += $(mooedit_printing_sources)
else
EXTRA_DIST += $(mooedit_printing_sources)
endif

if MOO_BUILD_LUA
mooedit_sources += $(lua_sources)
else
EXTRA_DIST += $(lua_sources)
endif

noinst_LTLIBRARIES = libmooedit.la
libmooedit_la_SOURCES = $(mooedit_sources)
nodist_libmooedit_la_SOURCES = $(nodist_mooedit_sources)

libmooedit_la_LIBADD = plugins/libmooeditplugins.la gtksourceview/libgtksourceview.la

AM_CFLAGS =			\
	-I..			\
	-I$(top_builddir)	\
	-I$(top_srcdir)/doc	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)
AM_OBJCFLAGS = $(AM_CFLAGS) $(MOO_OBJCFLAGS)

if MOO_OS_MINGW
AM_CFLAGS += $(MOO_WIN32_CFLAGS)
endif

if MOO_INSTALL_HEADERS
mooedit_includedir = $(moo_includedir)/mooedit
mooedit_include_HEADERS = $(mooedit_include_headers)
nodist_mooedit_include_HEADERS = $(nodist_mooedit_include_headers)
endif
