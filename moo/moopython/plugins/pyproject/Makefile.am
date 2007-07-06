ini_in_in_files = project-plugin.ini.desktop.in.in
@MOO_INI_IN_IN_RULE@
@MOO_INTLTOOL_INI_RULE@
ini_in_files = $(ini_in_in_files:.ini.desktop.in.in=.ini.desktop.in)
ini_files = $(ini_in_files:.ini.desktop.in=.ini)
CLEANFILES = $(ini_files) $(ini_in_files)

inidir = ${MOO_PLUGINS_DIR}
ini_DATA =				\
	$(ini_files)

plugindir = $(inidir)/project
nobase_plugin_DATA =			\
	project-plugin.py		\
	mprj/__init__.py		\
	mprj/manager.py			\
	mprj/project.py			\
	mprj/optdialog.py		\
	mprj/session.py			\
	mprj/settings.py		\
	mprj/simple.py			\
	mprj/simple.glade		\
	mprj/test.py			\
	mprj/utils.py			\
	mprj/config/_config.py		\
	mprj/config/_dict.py		\
	mprj/config/_group.py		\
	mprj/config/__init__.py		\
	mprj/config/_item.py		\
	mprj/config/_setting.py		\
	mprj/config/_utils.py		\
	mprj/config/view.py		\
	mprj/config/_xml.py

projectsdir = $(MOO_LIB_DIR)
nobase_projects_DATA =			\
	projects/c.py			\
	projects/cproj/config.py	\
	projects/cproj/__init__.py	\
	projects/cproj/optdialog.py	\
	projects/cproj/parser.py	\
	projects/cproj/options.glade	\
	projects/python.py		\
	projects/pyproj/config.py	\
	projects/pyproj/__init__.py	\
	projects/pyproj/optdialog.py	\
	projects/pyproj/options.glade

EXTRA_DIST =				\
    $(nobase_plugin_DATA)		\
    $(ini_in_in_files)			\
    $(nobase_projects_DATA)
