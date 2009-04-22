# -%- lang: makefile; indent-width: 8; use-tabs: true -%-
# UGLY_MK_TRIGGER = MOO_UI_FILES

ugly_ui2h = $(top_srcdir)/ugly/xml2h.sh

%-ui.h: %.xml $(ugly_ui2h)
	$(SHELL) $(ugly_ui2h) `basename "$*" .xml | sed -e "s/-/_/"`_ui_xml $< > $@.tmp && mv $@.tmp $@

ugly_ui_sources = $(MOO_UI_FILES:%.xml=%-ui.h)

BUILT_SOURCES += $(ugly_ui_sources)
CLEANFILES += $(ugly_ui_sources)
EXTRA_DIST += $(MOO_UI_FILES) $(ugly_ui2h)
