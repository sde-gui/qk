EXTRA_DIST = mooscript.y

# not using rule for auto-regenerating mooscript-yacc.[ch]
mooscript_yacc: $(srcdir)/mooscript.y
	touch mooscript_yacc
	bison -o $(srcdir)/mooscript-yacc.c -d $(srcdir)/mooscript.y

mooscript_headers =		\
	mooscript-context.h	\
	mooscript-func.h	\
	mooscript-node.h	\
	mooscript-parser.h	\
	mooscript-value.h	\
	mooscript-zenity.h

mooscript_sources =		\
	$(mooscript_headers)	\
	mooscript-builtin.c	\
	mooscript-context.c	\
	mooscript-func.c	\
	mooscript-node.c	\
	mooscript-parser.c	\
	mooscript-parser-priv.h	\
	mooscript-value.c	\
	mooscript-yacc.c	\
	mooscript-yacc.h	\
	mooscript-zenity.c

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

if MOO_INSTALL_LIB
mooscript_includedir = $(moo_includedir)/mooscript
mooscript_include_HEADERS = $(mooscript_headers)
endif
