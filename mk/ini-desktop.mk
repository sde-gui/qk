# -*- makefile -*-

# $(ini_desktop_in_in_files) should be a list of input *.ini.desktop.in.in files
# $(ini_desktop_files) is defined to be a list of generated *.ini files

%.ini.desktop.in: %.ini.desktop.in.in $(top_builddir)/config.status
	cd $(top_builddir) && $(SHELL) ./config.status --file=$(subdir)/$@

# %.ini: %.ini.desktop.in
@MOO_INTLTOOL_INI_RULE@

ini_desktop_in_files = $(ini_desktop_in_in_files:.ini.desktop.in.in=.ini.desktop.in)
ini_desktop_files = $(ini_desktop_in_files:.ini.desktop.in=.ini)

CLEANFILES += $(ini_desktop_files) $(ini_desktop_in_files)
EXTRA_DIST += $(ini_desktop_in_in_files)
