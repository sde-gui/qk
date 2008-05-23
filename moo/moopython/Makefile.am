SUBDIRS = plugins pygtk .

libmoomod_la_sources =		\
	moo-mod.c		\
	moopython-api.h		\
	moopython-utils.h	\
	moopython-utils.c	\
	moopython-loader.h	\
	moopython-loader.c

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

if !MOO_BUILD_MOO_MODULE
libmoopython_la_libadd =	\
	pygtk/libmoopygtk.la
endif

noinst_LTLIBRARIES =

if MOO_OS_UNIX

if MOO_PYTHON_BUILTIN
noinst_LTLIBRARIES += libmoopython.la
endif

if MOO_BUILD_PYTHON_PLUGIN
noinst_LTLIBRARIES += libpymoo.la
endif

if MOO_BUILD_MOO_MODULE
noinst_LTLIBRARIES += libmoomod.la
if MOO_PYTHON_BUILTIN
libmoomod_la_sources +=		\
	moopython-builtin.h	\
	moopython-builtin.c
endif
endif

else !MOO_OS_UNIX

noinst_LTLIBRARIES += libmoomod.la libpymoo.la

endif !MOO_OS_UNIX

AM_CFLAGS =			\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)	\
	$(PYTHON_INCLUDES)	\
	$(PYGTK_CFLAGS)		\
	$(MOO_W_NO_WRITE_STRINGS)

if MOO_OS_MINGW
AM_CFLAGS += -DMOO_PYTHON_MODULE_DLL_NAME=\"pymoo.dll\"
endif

libmoomod_la_SOURCES = $(libmoomod_la_sources)
libmoomod_la_LIBADD = pygtk/libmoopygtk.la $(libmoomod_la_libadd)

libpymoo_la_SOURCES = $(libpymoo_la_sources)

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
	codegen/argtypes_m.py		\
	codegen/codegen.py		\
	codegen/definitions.py		\
	codegen/defsparser.py		\
	codegen/docgen.py		\
	codegen/mergedefs.py		\
	codegen/mkskel.py		\
	codegen/override.py		\
	codegen/reversewrapper.py	\
	codegen/scmexpr.py
