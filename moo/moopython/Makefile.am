SUBDIRS = plugins pygtk .

libmoomod_la_sources =		\
	moo-mod.c		\
	moopython-api.h		\
	moopython-loader.h	\
	moopython-loader.c	\
	moopython-utils.h	\
	moopython-utils.c

libpymoo_la_sources =		\
	moopython-mod.c		\
	moopython-pygtkmod.h	\
	moopython-api.h		\
	moopython-loader.h	\
	moopython-loader.c	\
	moopython-utils.h	\
	moopython-utils.c

libmoopython_la_sources =	\
	moopython-builtin.h	\
	moopython-builtin.c	\
	moopython-api.h		\
	moopython-loader.h	\
	moopython-loader.c	\
	moopython-utils.h	\
	moopython-utils.c
libmoopython_la_libadd =	\
	pygtk/libmoopygtk.la

noinst_LTLIBRARIES =

if MOO_OS_UNIX

if MOO_INSTALL_LIB
noinst_LTLIBRARIES += libmoomod.la
if MOO_BUILD_PYTHON_PLUGIN
noinst_LTLIBRARIES += libpymoo.la
else
noinst_LTLIBRARIES += libmoopython.la
endif
else
noinst_LTLIBRARIES += libmoopython.la
endif

else !MOO_OS_UNIX

noinst_LTLIBRARIES += libmoomod25.la libmoomod24.la
noinst_LTLIBRARIES += libpymoo25.la libpymoo24.la

endif !MOO_OS_UNIX

AM_CFLAGS =			\
	-I..			\
	-I$(top_builddir)	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)	\
	$(PYTHON_CFLAGS)	\
	$(PYGTK_CFLAGS)

py25_cflags =			\
	-I..			\
	-I$(top_builddir)	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)	\
	$(PYTHON25_CFLAGS)	\
	$(PYGTK25_CFLAGS)	\
	-DMOO_PYTHON_MODULE_DLL_NAME=\"pymoo.dll\"

py24_cflags =			\
	-I..			\
	-I$(top_builddir)	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)	\
	$(PYTHON24_CFLAGS)	\
	$(PYGTK24_CFLAGS)	\
	-DMOO_PYTHON_MODULE_DLL_NAME=\"pymoo.dll\"

libmoomod_la_SOURCES = $(libmoomod_la_sources)
libmoomod_la_LIBADD = pygtk/libmoopygtk.la
libmoomod25_la_SOURCES = $(libmoomod_la_sources)
libmoomod25_la_LIBADD = pygtk/libmoopygtk25.la
libmoomod25_la_CFLAGS = $(py25_cflags)
libmoomod24_la_SOURCES = $(libmoomod_la_sources)
libmoomod24_la_LIBADD = pygtk/libmoopygtk24.la
libmoomod24_la_CFLAGS = $(py24_cflags)

libpymoo_la_SOURCES = $(libpymoo_la_sources)
libpymoo25_la_SOURCES = $(libpymoo_la_sources)
libpymoo25_la_CFLAGS = $(py25_cflags)
libpymoo24_la_SOURCES = $(libpymoo_la_sources)
libpymoo24_la_CFLAGS = $(py24_cflags)

libmoopython_la_SOURCES = $(libmoopython_la_sources)
libmoopython_la_LIBADD = $(libmoopython_la_libadd)

EXTRA_DIST =				\
	moo-mod.c			\
	moopython-mod.c			\
	moopython-builtin.h		\
	moopython-builtin.c		\
	moopython-api.h			\
	moopython-loader.h		\
	moopython-loader.c		\
	moopython-utils.h		\
	moopython-utils.c		\
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
	codegen/scmexpr.py		\
	$(ini_in_files)
