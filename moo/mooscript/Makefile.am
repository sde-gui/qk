EXTRA_DIST = mooscript.y

# not using rule for auto-regenerating mooscript-yacc.[ch]
mooscript_yacc: $(srcdir)/mooscript.y
	touch mooscript_yacc
	bison -o $(srcdir)/mooscript-yacc.m -d $(srcdir)/mooscript.y

mooscript.lo: mooscript-yacc.lo
mooscript-context-private.lo: mooscript-context.lo

mooscript_headers =			\
	mooscript-context.h		\
	mooscript-func.h		\
	mooscript-node.h		\
	mooscript-parser.h		\
	mooscript-value.h		\
	mooscript-zenity.h

mooscript_sources =			\
	$(mooscript_headers)		\
	mooscript-builtin.m		\
	mooscript-context-private.h	\
	mooscript-context.m		\
	mooscript-func-private.h	\
	mooscript-func.m		\
	mooscript-node-private.h	\
	mooscript-node.m		\
	mooscript-parser.m		\
	mooscript-parser-private.h	\
	mooscript-value-private.h	\
	mooscript-value.m		\
	mooscript-yacc.m		\
	mooscript-yacc.h		\
	mooscript-zenity.m

# BUILT_SOURCES =			\
# 	mooscript-yacc.c	\
# 	mooscript-yacc.h

noinst_LTLIBRARIES = libmooscript.la
libmooscript_la_SOURCES = $(mooscript_sources)

AM_CFLAGS =			\
	-I..			\
	-I$(top_builddir)	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)
AM_OBJCFLAGS = $(AM_CFLAGS) $(MOO_OBJCFLAGS)

if MOO_INSTALL_HEADERS
mooscript_includedir = $(moo_includedir)/mooscript
mooscript_include_HEADERS = $(mooscript_headers)
endif
