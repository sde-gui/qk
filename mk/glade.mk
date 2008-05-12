# -*- makefile -*-

# $(glade_files) should be a list of glade files
# $(glade_sources) is defined to be a list of generated *-glade.h files

%-glade.h: glade/%.glade $(top_srcdir)/moo/mooutils/xml2h.sh
	$(SHELL) $(top_srcdir)/moo/mooutils/xml2h.sh `basename "$*" | sed -e "s/-/_/"`_glade_xml $< > $@.tmp && mv $@.tmp $@
%-glade.h: %.glade $(top_srcdir)/moo/mooutils/xml2h.sh
	$(SHELL) $(top_srcdir)/moo/mooutils/xml2h.sh `basename "$*" | sed -e "s/-/_/"`_glade_xml $< > $@.tmp && mv $@.tmp $@

# glade_sources = $(patsubst glade/%.glade,%-glade.h,$(glade_files))
glade_sources = $(patsubst %.glade,%-glade.h,$(patsubst glade/%.glade,%-glade.h,$(glade_files)))

BUILT_SOURCES += $(glade_sources)
CLEANFILES += $(glade_sources)
EXTRA_DIST += $(glade_files)
