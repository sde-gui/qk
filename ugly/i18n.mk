# -%- lang: makefile; indent-width: 8; use-tabs: true -%-
# UGLY_MK_TRIGGER = INI_IN_IN_FILES|DESKTOP_IN_FILES

# $(INI_IN_IN_FILES) should be a list of input *.ini.in.in files
# $(DESKTOP_IN_FILES) should be a list of input *.desktop.in files

%.ini.in: %.ini.in.in $(top_builddir)/config.status
	cd $(top_builddir) && $(SHELL) ./config.status --file=$(subdir)/$@

# %.desktop: %.desktop.in
@MOO_INTLTOOL_DESKTOP_RULE@

# %.ini: %.ini.in
@MOO_INTLTOOL_INI_RULE@

ugly_ini_in_files = $(INI_IN_IN_FILES:.ini.in.in=.ini.in)
INI_FILES = $(ugly_ini_in_files:.ini.in=.ini)

DESKTOP_FILES = $(DESKTOP_IN_FILES:.desktop.in=.desktop)

CLEANFILES += $(INI_FILES) $(ugly_ini_in_files) $(DESKTOP_FILES)
EXTRA_DIST += $(INI_IN_IN_FILES) $(DESKTOP_IN_FILES)
