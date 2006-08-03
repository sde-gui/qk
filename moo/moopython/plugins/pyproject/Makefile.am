ini_in_files = project-plugin.ini.in
%.ini:   %.ini.in   $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po) ; LC_ALL=C $(INTLTOOL_MERGE) -d -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $< $@
ini_files = $(ini_in_files:.ini.in=.ini)

pluginsdir = $(MOO_LIB_DIR)/plugins
plugins_DATA =				\
	project-plugin.py		\
	$(ini_files)

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
    project-plugin.py			\
    $(ini_in_files)			\
    $(projects_DATA)			\
    $(mproj_DATA)			\
    $(cproj_DATA)			\
    $(ltxproj_DATA)
