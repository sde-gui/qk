EXTRA_DIST =
BUILT_SOURCES =
CLEANFILES =

AM_CPPFLAGS = $(MOO_CPPFLAGS) -I$(top_srcdir)/doc -Imooutils/glade
AM_CFLAGS = $(MOO_CFLAGS)
AM_CXXFLAGS = $(MOO_CXXFLAGS)

moo_sources =
built_moo_sources =
moo_libadd =

plugins_sources =
built_plugins_sources =

moo_srcdir = $(srcdir)
moo_builddir = .

moo_sources += moo-config.h

EXTRA_DIST += marshals.list
built_moo_sources += marshals.h
marshals.h: marshals.list
	$(GLIB_GENMARSHAL) --prefix=_moo_marshal --header $(srcdir)/marshals.list > marshals.h.tmp \
		&& mv marshals.h.tmp marshals.h

include xdgmime/Makefile.incl
include mooutils/Makefile.incl
include moolua/Makefile.incl
include moofileview/Makefile.incl
include gtksourceview/Makefile.incl
include mooedit/Makefile.incl
include eggsmclient/Makefile.incl
include mooapp/Makefile.incl
include mooscript/Makefile.incl
include plugins/Makefile.incl
include medit-app/Makefile.incl

BUILT_SOURCES += $(built_moo_sources) $(built_plugin_sources)
CLEANFILES += $(built_moo_sources) $(built_plugin_sources)

# glade/%-gxml.h: glade/%.glade $(top_srcdir)/tools/glade2c.py
# 	$(MKDIR_P) glade
# 	$(PYTHON) $(top_srcdir)/tools/glade2c.py $< > $@.tmp && mv $@.tmp $@

%-gxml.h: mooutils/glade/%.glade $(top_srcdir)/tools/glade2c.py
	$(PYTHON) $(top_srcdir)/tools/glade2c.py $< > $@.tmp && mv $@.tmp $@

%-gxml.h: moofileview/glade/%.glade $(top_srcdir)/tools/glade2c.py
	$(PYTHON) $(top_srcdir)/tools/glade2c.py $< > $@.tmp && mv $@.tmp $@

%-gxml.h: mooedit/glade/%.glade $(top_srcdir)/tools/glade2c.py
	$(PYTHON) $(top_srcdir)/tools/glade2c.py $< > $@.tmp && mv $@.tmp $@

%-gxml.h: mooapp/glade/%.glade $(top_srcdir)/tools/glade2c.py
	$(PYTHON) $(top_srcdir)/tools/glade2c.py $< > $@.tmp && mv $@.tmp $@

%-gxml.h: plugins/glade/%.glade $(top_srcdir)/tools/glade2c.py
	$(PYTHON) $(top_srcdir)/tools/glade2c.py $< > $@.tmp && mv $@.tmp $@

%-gxml.h: plugins/usertools/glade/%.glade $(top_srcdir)/tools/glade2c.py
	$(PYTHON) $(top_srcdir)/tools/glade2c.py $< > $@.tmp && mv $@.tmp $@

# %-ui.h: %.xml $(top_srcdir)/tools/xml2h.py
# 	$(PYTHON) $(top_srcdir)/tools/xml2h.py $< $@.tmp $*_ui_xml && mv $@.tmp $@

%-ui.h: moofileview/%.xml $(top_srcdir)/tools/xml2h.py
	$(PYTHON) $(top_srcdir)/tools/xml2h.py $< $@.tmp $*_ui_xml && mv $@.tmp $@

%-ui.h: mooedit/%.xml $(top_srcdir)/tools/xml2h.py
	$(PYTHON) $(top_srcdir)/tools/xml2h.py $< $@.tmp $*_ui_xml && mv $@.tmp $@
