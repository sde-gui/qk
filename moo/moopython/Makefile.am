SUBDIRS = plugins pygtk .

noinst_LTLIBRARIES =

if MOO_INSTALL_LIB
noinst_LTLIBRARIES += libmoomod.la
if MOO_BUILD_PYTHON_MODULE
noinst_LTLIBRARIES += libpymoo.la
else
noinst_LTLIBRARIES += libmoopython.la
endif
else
noinst_LTLIBRARIES += libmoopython.la
endif

libmoomod_la_SOURCES =		\
	moo-mod.c		\
	moopython-api.h		\
	moopython-loader.h	\
	moopython-loader.c	\
	moopython-utils.h	\
	moopython-utils.c
libmoomod_la_LIBADD =		\
	pygtk/libmoopygtk.la

libpymoo_la_SOURCES =		\
	moopython-mod.c		\
	moopython-api.h		\
	moopython-loader.h	\
	moopython-loader.c	\
	moopython-utils.h	\
	moopython-utils.c

libmoopython_la_SOURCES =	\
	moopython.h		\
	moopython.c		\
	moopython-api.h		\
	moopython-loader.h	\
	moopython-loader.c	\
	moopython-utils.h	\
	moopython-utils.c
libmoopython_la_LIBADD =	\
	pygtk/libmoopygtk.la

AM_CFLAGS =			\
	-I..			\
	-I$(top_builddir)	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)	\
	$(PYTHON_CFLAGS)	\
	$(PYGTK_CFLAGS)		\
	$(lib_define)

EXTRA_DIST =				\
	moo-mod.c			\
	moopython-mod.c			\
	moopython.h			\
	moopython.c			\
	moopython-api.h			\
	moopython-loader.h		\
	moopython-loader.c		\
	moopython-utils.h		\
	moopython-utils.c		\
	pymoo.ini			\
	codegen/__init__.py		\
	codegen/argtypes.py		\
	codegen/codegen.py		\
	codegen/definitions.py		\
	codegen/defsparser.py		\
	codegen/docgen.py		\
	codegen/mergedefs.py		\
	codegen/mkskel.py		\
	codegen/override.py		\
	codegen/reversewrapper.py	\
	codegen/scmexpr.py
