# -*- makefile -*-

# $(enum_prefix), optional, prefix FOO in FOO_TYPE_SOMETHING
# $(enum_file) must be the name of generated file
# $(enum_headers) must be the list of headers to parse for enums

enum_hfile = $(enum_file).h
enum_cfile = $(enum_file).c
stamp_enum_hfile = stamp-$(enum_hfile)
stamp_enum_cfile = stamp-$(enum_cfile)

use_enum_prefix = $(if $(enum_prefix),$(enum_prefix),MOO)

BUILT_SOURCES += $(stamp_enum_hfile) $(stamp_enum_cfile)
CLEANFILES += $(stamp_enum_hfile) $(stamp_enum_cfile)

$(stamp_enum_hfile): $(enum_headers) Makefile
	( HGUARD=`echo $(enum_hfile) | tr '[a-z.\-]' '[A-Z__]'` && cd $(srcdir) && \
          glib-mkenums --fhead "#ifndef $$HGUARD\n#define $$HGUARD\n\n#include <glib-object.h>\n" \
                       --fhead "\nG_BEGIN_DECLS\n\n\n" \
                       --fprod "/* enumerations from @filename@ */\n" \
                       --vhead "GType @enum_name@_get_type (void) G_GNUC_CONST;\n#define $(use_enum_prefix)_TYPE_@ENUMSHORT@ (@enum_name@_get_type())\n\n" \
                       --ftail "\nG_END_DECLS\n\n#endif /* $$HGUARD */" \
		$(enum_headers) ) > $(enum_hfile).tmp \
	&& (cmp -s $(enum_hfile).tmp $(srcdir)/$(enum_hfile) || cp $(enum_hfile).tmp $(srcdir)/$(enum_hfile) ) \
	&& rm -f $(enum_hfile).tmp \
	&& echo timestamp > $(@F)
$(stamp_enum_cfile): $(enum_headers) Makefile
	( cd $(srcdir) && glib-mkenums \
			--fhead "#include \"$(enum_hfile)\"\n\n" \
			--fprod "#include \"@filename@\"\n\n" \
			--vhead "GType\n@enum_name@_get_type (void)\n{\n    static GType etype;\n    if (G_UNLIKELY (!etype))\n    {\n        static const G@Type@Value values[] = {" \
			--vprod "            { @VALUENAME@, (char*) \"@VALUENAME@\", (char*) \"@valuenick@\" }," \
			--vtail "            { 0, NULL, NULL }\n        };\n        etype = g_@type@_register_static (\"@EnumName@\", values);\n    }\n    return etype;\n}\n\n" \
		$(enum_headers) ) > $(enum_cfile).tmp \
	&& (cmp -s $(enum_cfile).tmp $(srcdir)/$(enum_cfile) || cp $(enum_cfile).tmp $(srcdir)/$(enum_cfile) ) \
	&& rm -f $(enum_cfile).tmp \
	&& echo timestamp > $(@F)
