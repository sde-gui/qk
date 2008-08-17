# -%- lang: makefile; indent-width: 8; use-tabs: true -%-
# UGLY_MK_TRIGGER = GLADE_FILES

ugly_xml2h = $(top_srcdir)/ugly/xml2h.sh

%-glade.h: glade/%.glade $(ugly_xml2h)
	$(SHELL) $(ugly_xml2h) `basename "$*" | sed -e "s/-/_/"`_glade_xml $< > $@.tmp && mv $@.tmp $@
%-glade.h: %.glade $(ugly_xml2h)
	$(SHELL) $(ugly_xml2h) `basename "$*" | sed -e "s/-/_/"`_glade_xml $< > $@.tmp && mv $@.tmp $@

ugly_glade_sources_1 = $(GLADE_FILES:%.glade=%-glade.h)
ugly_glade_sources = $(ugly_glade_sources_1:glade/%-glade.h=%-glade.h)

BUILT_SOURCES += $(ugly_glade_sources)
CLEANFILES += $(ugly_glade_sources)
EXTRA_DIST += $(GLADE_FILES) $(ugly_xml2h)
