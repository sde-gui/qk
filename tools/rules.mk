glade/%-gxml.h: glade/%.glade $(top_srcdir)/tools/glade2c.py
	$(MKDIR_P) glade
	$(PYTHON) $(top_srcdir)/tools/glade2c.py $< > $@.tmp && mv $@.tmp $@

%-ui.h: %.xml $(top_srcdir)/tools/xml2h.py
	$(PYTHON) $(top_srcdir)/tools/xml2h.py $< $@.tmp $*_ui_xml && mv $@.tmp $@

# -%- lang:makefile; strip:true; indent-width:8; use-tabs:true -%-
