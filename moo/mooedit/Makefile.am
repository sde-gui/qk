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
	mooedit-lua-api.c		\
	mooedit-lua.c			\
	mooedit-lua.h

completion_sources =			\
	moocompletionsimple.c		\
	moocompletionsimple.h		\
	mootextcompletion.c		\
	mootextcompletion.h		\
	mootextpopup.c			\
	mootextpopup.h

mooedit_sources =			\
	$(moocommand_stuff)		\
	$(completion_sources)		\
	moocmdview.c			\
	moocmdview.h			\
	moocommand-exe.c		\
	moocommand-exe.h		\
	moocommand-private.h		\
	moocommand.c			\
	moocommand.h			\
	moocommanddisplay.c		\
	moocommanddisplay.h		\
	mooedit-accels.h		\
	mooedit-bookmarks.c		\
	mooedit-bookmarks.h		\
	mooedit-enums.c			\
	mooedit-enums.h			\
	mooedit-private.h		\
	mooedit.c			\
	mooedit.h			\
	mooeditaction-factory.c		\
	mooeditaction-factory.h		\
	mooeditaction.c			\
	mooeditaction.h			\
	mooeditconfig.c			\
	mooeditconfig.h			\
	mooeditdialogs.c		\
	mooeditdialogs.h		\
	mooeditfileops.c		\
	mooeditfileops.h		\
	mooeditfiltersettings.c		\
	mooeditfiltersettings.h		\
	mooeditinput.c			\
	mooeditor-private.h		\
	mooeditor.c			\
	mooeditor.h			\
	mooeditprefs.c			\
	mooeditprefs.h			\
	mooeditprefspage.c		\
	mooeditwindow.c			\
	mooeditwindow.h			\
	moofold.c			\
	moofold.h			\
	mooindenter.c			\
	mooindenter.h			\
	mookeyfile.c			\
	mookeyfile.h			\
	moolang-private.h		\
	moolang.c			\
	moolang.h			\
	moolangmgr-private.h		\
	moolangmgr.c			\
	moolangmgr.h			\
	moolinebuffer.c			\
	moolinebuffer.h			\
	moolinemark.c			\
	moolinemark.h			\
	moolineview.c			\
	moolineview.h			\
	moooutputfilter.c		\
	moooutputfilter.h		\
	moooutputfilterregex.c		\
	moooutputfilterregex.h		\
	mooplugin-loader.c		\
	mooplugin-loader.h		\
	mooplugin-macro.h		\
	mooplugin.c			\
	mooplugin.h			\
	mootext-private.h		\
	mootextbox.c			\
	mootextbox.h			\
	mootextbtree.c			\
	mootextbtree.h			\
	mootextbuffer.c			\
	mootextbuffer.h			\
	mootextfind.c			\
	mootextfind.h			\
	mootextiter.h			\
	mootextsearch-private.h		\
	mootextsearch.c			\
	mootextsearch.h			\
	mootextstylescheme.c		\
	mootextstylescheme.h		\
	mootextview-private.h		\
	mootextview.c			\
	mootextview.h			\
	moousertools-prefs.c		\
	moousertools-prefs.h		\
	moousertools.c			\
	moousertools.h

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

EXTRA_DIST =

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
include ../../mk/glade.mk

ui_files = medit.ui mooedit.ui
include ../../mk/ui.mk

mime_in_files = moo.xml.in
include ../../mk/mime.mk

EXTRA_DIST +=				\
	$(mooedit_tools_DATA)		\
	$(mooedit_lua_DATA)

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

libmooedit_la_LIBADD = plugins/libmooeditplugins.la gtksourceview/libgtksourceview.la

AM_CFLAGS =			\
	-I..			\
	-I$(top_builddir)	\
	-I$(top_srcdir)/doc	\
	-I$(top_builddir)/doc	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)
AM_OBJCFLAGS = $(AM_CFLAGS)

if MOO_OS_MINGW
AM_CFLAGS += $(MOO_WIN32_CFLAGS)
endif

include ../../mk/marshals.mk
