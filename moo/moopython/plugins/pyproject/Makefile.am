ini_in_files = project-plugin.ini.desktop.in
%.ini:   %.ini.desktop.in   $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po) ; LC_ALL=C $(INTLTOOL_MERGE) -d -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $< $@
ini_files = $(ini_in_files:.ini.desktop.in=.ini)
CLEANFILES = $(ini_files)

inidir = $(MOO_LIB_DIR)/plugins
ini_DATA =				\
	$(ini_files)

plugindir = $(MOO_LIB_DIR)/plugins/project
plugin_DATA =				\
	project-plugin.py

mprojdir = $(plugindir)/mproj
mproj_DATA =				\
	mproj/config.py			\
	mproj/configxml.py		\
	mproj/__init__.py		\
	mproj/manager.py		\
	mproj/option.py			\
	mproj/project.py		\
	mproj/session.py		\
	mproj/simple.py			\
	mproj/utils.py

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
    $(plugin_DATA)			\
    $(ini_in_files)			\
    $(projects_DATA)			\
    $(mproj_DATA)			\
    $(cproj_DATA)			\
    $(ltxproj_DATA)
