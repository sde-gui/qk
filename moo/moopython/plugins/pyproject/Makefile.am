pluginsdir = $(MOO_LIB_DIR)/plugins
plugins_DATA =				\
	project-plugin.py

mprojdir = $(pluginsdir)/lib/mproj
mproj_DATA =				\
	lib/mproj/config.py		\
	lib/mproj/configxml.py		\
	lib/mproj/__init__.py		\
	lib/mproj/manager.py		\
	lib/mproj/option.py		\
	lib/mproj/project.py		\
	lib/mproj/session.py		\
	lib/mproj/simple.py		\
	lib/mproj/utils.py

projectsdir = $(MOO_LIB_DIR)/projects
projects_DATA =				\
	projects/c.py			\
	projects/latex.py

cprojdir = $(projectsdir)/cproj
cproj_DATA =				\
	projects/cproj/config.py	\
	projects/cproj/__init__.py	\
	projects/cproj/parser.py

ltxprojdir = $(projectsdir)/ltxproj
ltxproj_DATA =				\
	projects/ltxproj/config.py	\
	projects/ltxproj/__init__.py

EXTRA_DIST =				\
    $(plugins_DATA)			\
    $(projects_DATA)			\
    $(mproj_DATA)			\
    $(cproj_DATA)			\
    $(ltxproj_DATA)
