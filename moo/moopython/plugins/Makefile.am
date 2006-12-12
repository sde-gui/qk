SUBDIRS = pyproject

moopython_pluginsdir = $(MOO_LIB_DIR)/plugins
moopython_plugins_libdir = $(MOO_LIB_DIR)/plugins/lib

ini_in_in_files =			\
	terminal.ini.desktop.in.in	\
	python.ini.desktop.in.in

ini_in_files = $(ini_in_in_files:.ini.desktop.in.in=.ini.desktop.in)
ini_files = $(ini_in_files:.ini.desktop.in=.ini)

@MOO_INI_IN_IN_RULE@
@MOO_INTLTOOL_INI_RULE@
CLEANFILES = $(ini_files) $(ini_in_files)

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
    $(ini_in_in_files)
