# -*- makefile -*-

# $(ui_files) should be a list of ui xml files
# $(ui_sources) is defined to be a list of generated *-ui.h files

%-ui.h: %.xml $(MOO_XML2H)
	$(SHELL) $(MOO_XML2H) `basename "$*" .xml | sed -e "s/-/_/"`_ui_xml $< > $@.tmp && mv $@.tmp $@

ui_sources = $(patsubst %.xml,%-ui.h,$(ui_files))
BUILT_SOURCES += $(ui_sources)
CLEANFILES += $(ui_sources)
EXTRA_DIST += $(ui_files)
