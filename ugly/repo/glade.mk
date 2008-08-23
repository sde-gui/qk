# -%- lang: makefile; indent-width: 8; use-tabs: true -%-
# UGLY_MK_TRIGGER = GLADE_FILES

ugly_glade2h = $(top_srcdir)/ugly/glade2h.sh
ugly_xml2h = $(top_srcdir)/ugly/xml2h.sh
ugly_glade_mk = $(top_srcdir)/ugly/glade.mk

%-glade.h: glade/%.glade $(ugly_xml2h) $(ugly_glade2h) $(ugly_glade_mk)
	@mkdir -p `dirname $@`
	$(ugly_glade2h) $< > $@.tmp && mv $@.tmp $@
%-glade.h: %.glade $(ugly_xml2h) $(ugly_glade2h) $(ugly_glade_mk)
	@mkdir -p `dirname $@`
	$(ugly_glade2h) $< > $@.tmp && mv $@.tmp $@

ugly_glade_sources_1 = $(GLADE_FILES:%.glade=%-glade.h)
ugly_glade_sources = $(ugly_glade_sources_1:glade/%-glade.h=%-glade.h)

BUILT_SOURCES += $(ugly_glade_sources)
CLEANFILES += $(ugly_glade_sources)
EXTRA_DIST += $(GLADE_FILES) $(ugly_xml2h) $(ugly_glade2h)
