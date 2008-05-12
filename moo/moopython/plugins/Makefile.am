DIST_SUBDIRS = pyproject
SUBDIRS = .

if MOO_ENABLE_PROJECT
SUBDIRS += pyproject
endif

moopython_pluginsdir = ${MOO_PLUGINS_DIR}
plugins_libdir = ${MOO_PLUGINS_DIR}/lib

EXTRA_DIST =
CLEANFILES =
ini_desktop_in_in_files =		\
	terminal.ini.desktop.in.in	\
	python.ini.desktop.in.in	\
	pycmd.ini.desktop.in.in
include ../../../mk/ini-desktop.mk

plugins =			\
	terminal.py		\
	python.py		\
	pycmd.py

moopython_plugins_DATA =	\
	$(ini_desktop_files)	\
	$(plugins)

plugins_lib_DATA =		\
	lib/pyconsole.py	\
	lib/insert_date_and_time.py

nobase_plugins_lib_DATA =	\
	medit/__init__.py	\
	medit/runpython.py

EXTRA_DIST +=			\
    $(nobase_plugins_lib_DATA)	\
    $(plugins_lib_DATA)		\
    $(plugins)
