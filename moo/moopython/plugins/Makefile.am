SUBDIRS = pyproject

moopython_pluginsdir = $(MOO_LIB_DIR)/plugins
moopython_plugins_libdir = $(MOO_LIB_DIR)/plugins/lib

ini_in_files = console.ini.in python.ini.in
%.ini:   %.ini.in   $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po) ; LC_ALL=C $(INTLTOOL_MERGE) -d -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $< $@
ini_files = $(ini_in_files:.ini.in=.ini)

plugins =			\
	console.py		\
	python.py

moopython_plugins_DATA =	\
	$(ini_files)		\
	$(plugins)

moopython_plugins_lib_DATA =	\
	pyconsole.py

EXTRA_DIST =				\
    $(moopython_plugins_lib_DATA)	\
    $(plugins)				\
    $(ini_in_files)

CLEANFILES = $(ini_files)
