ini_in_files = project-plugin.ini.desktop.in
@MOO_INTLTOOL_INI_RULE@
ini_files = $(ini_in_files:.ini.desktop.in=.ini)
CLEANFILES = $(ini_files)

inidir = $(MOO_LIB_DIR)/plugins
ini_DATA =				\
	$(ini_files)

plugindir = $(MOO_LIB_DIR)/plugins/project
plugin_DATA =				\
	project-plugin.py

mprjdir = $(plugindir)
nobase_mprj_DATA =			\
	mprj/__init__.py		\
	mprj/manager.py			\
	mprj/project.py			\
	mprj/session.py			\
	mprj/settings.py		\
	mprj/simple.py			\
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
	projects/latex.py		\
	projects/cproj/config.py	\
	projects/cproj/__init__.py	\
	projects/cproj/optdialog.py	\
	projects/cproj/parser.py	\
	projects/cproj/view.py		\
	projects/cproj/options.glade	\
	projects/ltxproj/config.py	\
	projects/ltxproj/__init__.py

EXTRA_DIST =				\
    $(plugin_DATA)			\
    $(ini_in_files)			\
    $(nobase_mprj_DATA)			\
    $(nobase_projects_DATA)
