SUBDIRS = plugins pygtk .

moopython_sources =		\
	moopython-utils.c	\
	moopython-utils.h

EXTRA_DIST =			\
	mooplugin-python.c	\
	mooplugin-python.h

if MOO_BUILD_EDIT
moopython_sources +=		\
	mooplugin-python.c	\
	mooplugin-python.h
endif

noinst_LTLIBRARIES = libmoopython.la
libmoopython_la_LIBADD = pygtk/libmoopygtk.la
libmoopython_la_SOURCES = $(moopython_sources)
AM_CFLAGS =			\
	-I..			\
	-I$(top_builddir)	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)	\
	$(PYTHON_INCLUDES)	\
	$(PYGTK_CFLAGS)

EXTRA_DIST +=			     \
	codegen/__init__.py          \
	codegen/argtypes.py          \
	codegen/codegen.py           \
	codegen/definitions.py       \
	codegen/defsparser.py        \
	codegen/docgen.py            \
	codegen/mergedefs.py         \
	codegen/mkskel.py            \
	codegen/override.py          \
	codegen/reversewrapper.py    \
	codegen/scmexpr.py
