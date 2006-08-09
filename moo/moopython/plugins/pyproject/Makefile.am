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

mprjdir = $(plugindir)/mprj
mprj_DATA =
# 	mprj/config.py			\
# 	mprj/configxml.py		\
# 	mprj/__init__.py		\
# 	mprj/manager.py			\
# 	mprj/option.py			\
# 	mprj/project.py			\
# 	mprj/session.py			\
# 	mprj/simple.py			\
# 	mprj/utils.py

# projectsdir = $(MOO_LIB_DIR)/projects
# projects_DATA =				\
# 	projects/c.py			\
# 	projects/latex.py
#
# cprojdir = $(projectsdir)/cproj
# cproj_DATA =				\
# 	projects/cproj/config.py	\
# 	projects/cproj/__init__.py	\
# 	projects/cproj/parser.py
#
# ltxprojdir = $(projectsdir)/ltxproj
# ltxproj_DATA =				\
# 	projects/ltxproj/config.py	\
# 	projects/ltxproj/__init__.py

# EXTRA_DIST =				\
#     $(plugin_DATA)			\
#     $(ini_in_files)			\
#     $(projects_DATA)			\
#     $(mprj_DATA)			\
#     $(cproj_DATA)			\
#     $(ltxproj_DATA)
