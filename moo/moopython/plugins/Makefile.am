SUBDIRS = pyproject

moopython_pluginsdir = $(MOO_LIB_DIR)/plugins
moopython_plugins_libdir = $(MOO_LIB_DIR)/plugins/lib

ini_in_files = terminal.ini.desktop.in python.ini.desktop.in
@MOO_INTLTOOL_INI_RULE@
ini_files = $(ini_in_files:.ini.desktop.in=.ini)
CLEANFILES = $(ini_files)

plugins =			\
	terminal.py		\
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
