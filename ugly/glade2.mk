# -%- lang: makefile; indent-width: 8; use-tabs: true -%-
# UGLY_MK_TRIGGER = GLADE2_FILES

ugly_glade2c = $(top_srcdir)/ugly/glade2c.py
ugly_glade2_mk = $(top_srcdir)/ugly/glade2.mk

glade/%-gxml.h: glade/%.glade $(ugly_glade2c) $(ugly_glade2_mk)
	@mkdir -p `dirname $@`
	$(PYTHON) $(ugly_glade2c) $< > $@.tmp && mv $@.tmp $@
%-gxml.h: %.glade $(ugly_glade2c) $(ugly_glade2_mk)
	@mkdir -p `dirname $@`
	$(PYTHON) $(ugly_glade2c) $< > $@.tmp && mv $@.tmp $@

ugly_glade2_sources = $(GLADE2_FILES:%.glade=%-gxml.h)

BUILT_SOURCES += $(ugly_glade2_sources)
EXTRA_DIST += $(GLADE2_FILES) $(ugly_glade2c) $(ugly_glade2_sources)
