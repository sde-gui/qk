# -%- lang: makefile; indent-width: 8; use-tabs: true -%-
# UGLY_MK_TRIGGER = ENUM_HDRS
# ENUM_PREFIX

ugly_enum_hfile = $(ENUM_MODULE)-enums.h
ugly_enum_cfile = $(ENUM_MODULE)-enums.c
ugly_stamp_enum_hfile = stamp-$(ugly_enum_hfile)
ugly_stamp_enum_cfile = stamp-$(ugly_enum_cfile)

ugly_enum_prefix = $(ENUM_PREFIX)

BUILT_SOURCES += $(ugly_stamp_enum_hfile) $(ugly_stamp_enum_cfile)
CLEANFILES += $(ugly_stamp_enum_hfile) $(ugly_stamp_enum_cfile)

$(ugly_stamp_enum_hfile): $(ENUM_HDRS) Makefile
	( HGUARD=`echo $(ugly_enum_hfile) | tr '[a-z.\-]' '[A-Z__]'` && cd $(srcdir) && \
	  $(GLIB_MKENUMS) --fhead "#ifndef $$HGUARD\n#define $$HGUARD\n\n#include <glib-object.h>\n" \
	               --fhead "\nG_BEGIN_DECLS\n\n\n" \
	               --fprod "/* enumerations from @filename@ */\n" \
	               --vhead "GType @enum_name@_get_type (void) G_GNUC_CONST;\n#define $(ugly_enum_prefix)_TYPE_@ENUMSHORT@ (@enum_name@_get_type())\n\n" \
	               --ftail "\nG_END_DECLS\n\n#endif /* $$HGUARD */" \
		$(ENUM_HDRS) ) > $(ugly_enum_hfile).tmp \
	&& (cmp -s $(ugly_enum_hfile).tmp $(srcdir)/$(ugly_enum_hfile) || cp $(ugly_enum_hfile).tmp $(srcdir)/$(ugly_enum_hfile) ) \
	&& rm -f $(ugly_enum_hfile).tmp \
	&& echo timestamp > $(@F)
$(ugly_stamp_enum_cfile): $(ENUM_HDRS) Makefile
	( cd $(srcdir) && $(GLIB_MKENUMS) \
			--fhead "#include \"$(ugly_enum_hfile)\"\n\n" \
			--fprod "#include \"@filename@\"\n\n" \
			--vhead "GType\n@enum_name@_get_type (void)\n{\n    static GType etype;\n    if (G_UNLIKELY (!etype))\n    {\n        static const G@Type@Value values[] = {" \
			--vprod "            { @VALUENAME@, (char*) \"@VALUENAME@\", (char*) \"@valuenick@\" }," \
			--vtail "            { 0, NULL, NULL }\n        };\n        etype = g_@type@_register_static (\"@EnumName@\", values);\n    }\n    return etype;\n}\n\n" \
		$(ENUM_HDRS) ) > $(ugly_enum_cfile).tmp \
	&& (cmp -s $(ugly_enum_cfile).tmp $(srcdir)/$(ugly_enum_cfile) || cp $(ugly_enum_cfile).tmp $(srcdir)/$(ugly_enum_cfile) ) \
	&& rm -f $(ugly_enum_cfile).tmp \
	&& echo timestamp > $(@F)
