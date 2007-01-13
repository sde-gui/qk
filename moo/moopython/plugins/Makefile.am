SUBDIRS = pyproject

moopython_pluginsdir = ${MOO_PLUGINS_DIR}
plugins_libdir = ${MOO_PLUGINS_DIR}/lib

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

nobase_plugins_lib_DATA =	\
	pyconsole.py		\
	medit/__init__.py	\
	medit/runpython.py

EXTRA_DIST =			\
    $(nobase_plugins_lib_DATA)	\
    $(plugins)			\
    $(ini_in_in_files)
