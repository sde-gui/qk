AM_CFLAGS =				\
	-I../../..			\
	-I$(top_builddir)		\
	$(MOO_CFLAGS)			\
	$(MOO_DEBUG_CFLAGS)

noinst_LTLIBRARIES = libgoocanvas.la

libgoocanvas_public_headers =		\
	goocanvasellipse.h		\
	goocanvasgroup.h		\
	goocanvasimage.h		\
	goocanvasitem.h			\
	goocanvasitemmodel.h		\
	goocanvasitemsimple.h		\
	goocanvaspolyline.h		\
	goocanvaspath.h			\
	goocanvasrect.h			\
	goocanvasstyle.h		\
	goocanvastable.h		\
	goocanvastext.h			\
	goocanvasutils.h		\
	goocanvaswidget.h		\
	goocanvas.h

libgoocanvas_la_SOURCES =		\
	$(libgoocanvas_public_headers)	\
	goocanvasatk.h			\
	goocanvasatk.c			\
	goocanvasellipse.c		\
	goocanvasenumtypes.c		\
	goocanvasgroup.c		\
	goocanvasimage.c		\
	goocanvasitem.c			\
	goocanvasitemmodel.c		\
	goocanvasitemsimple.c		\
	goocanvaspolyline.c		\
	goocanvaspath.c			\
	goocanvasprivate.h		\
	goocanvasrect.c			\
	goocanvasstyle.c		\
	goocanvastable.c		\
	goocanvastext.c			\
	goocanvasutils.c		\
	goocanvaswidget.c		\
	goocanvas.c			\
	goocanvasmarshal.h		\
	goocanvasenumtypes.h

BUILT_SOURCES =				\
	goocanvasenumtypes.h		\
	goocanvasenumtypes.c

CLEANFILES =				\
	stamp-goocanvasenumtypes.h	\
	stamp-goocanvasenumtypes.c
DISTCLEANFILES =			\
	goocanvasenumtypes.h		\
	goocanvasenumtypes.c

goocanvasenumtypes.h: stamp-goocanvasenumtypes.h
	@true
goocanvasenumtypes.c: stamp-goocanvasenumtypes.c
	@true

stamp-goocanvasenumtypes.h: $(libgoocanvas_public_headers) Makefile
	(cd $(srcdir) && glib-mkenums \
		--fhead "#ifndef __GOO_CANVAS_ENUM_TYPES_H__\n#define __GOO_CANVAS_ENUM_TYPES_H__\n\n#include <mooutils/moocompat.h>\n#include <glib-object.h>\n\nG_BEGIN_DECLS\n" \
		--fprod "/* Enumerations from \"@filename@\" */\n" \
		--vhead "GType @enum_name@_get_type (void);\n#define GOO_TYPE_@ENUMSHORT@ (@enum_name@_get_type())\n\n\n" \
		--ftail "G_END_DECLS\n\n#endif /* __GOO_CANVAS_ENUM_TYPES_H__ */" \
		$(libgoocanvas_public_headers)) >> xgen-gtbh \
	&& (cmp -s xgen-gtbh goocanvasenumtypes.h || cp xgen-gtbh goocanvasenumtypes.h ) \
	&& rm -f xgen-gtbh \
	&& echo timestamp > $(@F)

stamp-goocanvasenumtypes.c: $(libgoocanvas_public_headers) Makefile
	(cd $(srcdir) && glib-mkenums \
		--fhead "#include <glib-object.h>\n" \
		--fhead "#include \"goocanvas.h\"\n" \
		--fprod "\n/* Enumerations from \"@filename@\" */" \
		--vhead "GType\n@enum_name@_get_type (void)\n{\n  static GType etype = 0;\n if( etype == 0 ) \n  {\n    static const G@Type@Value values[] = {" \
		--vprod "      { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
		--vtail "      { 0, NULL, NULL }\n    };\n    etype = g_@type@_register_static (\"@EnumName@\", values );\n  }\n  return etype;\n}\n" \
		$(libgoocanvas_public_headers)) > xgen-gtbc \
	&& (cmp -s xgen-gtbc goocanvasenumtypes.c || cp xgen-gtbc goocanvasenumtypes.c)  \
	&& rm -f xgen-gtbc \
	&& echo timestamp > $(@F)
