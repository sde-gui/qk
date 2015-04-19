LIST(APPEND plugins_sources
	plugins/usertools/moousertools.c
	plugins/usertools/moousertools.h
	plugins/usertools/moousertools-prefs.c
	plugins/usertools/moousertools-prefs.h
	plugins/usertools/moocommand.c
	plugins/usertools/moocommand.h
	plugins/usertools/moocommand-private.h
	plugins/usertools/moocommanddisplay.c
	plugins/usertools/moocommanddisplay.h
	plugins/usertools/moooutputfilterregex.c
	plugins/usertools/moooutputfilterregex.h
	plugins/usertools/moousertools-enums.h
	plugins/usertools/moousertools-enums.c
	plugins/usertools/moocommand-exe.c
	plugins/usertools/moocommand-exe.h
	plugins/usertools/moocommand-script.cpp
	plugins/usertools/moocommand-script.h
)

foreach(input_file
	plugins/usertools/glade/mooedittools-exe.glade
	plugins/usertools/glade/mooedittools-script.glade
	plugins/usertools/glade/moousertools.glade
)
    ADD_GXML(${input_file})
endforeach(input_file)

XML2H(plugins/usertools/lua-tool-setup.lua plugins/usertools/lua-tool-setup.h LUA_TOOL_SETUP_LUA)

# EXTRA_DIST +=						\
# 	plugins/usertools/python-tool-setup.py
# if MOO_ENABLE_PYTHON
# built_plugins_sources += plugins/usertools/python-tool-setup.h
# plugins/usertools/python-tool-setup.h: plugins/usertools/python-tool-setup.py $(top_srcdir)/tools/xml2h.py
# 	$(AM_V_at)$(MKDIR_P) plugins/usertools
# 	$(AM_V_GEN)$(MOO_PYTHON) $(top_srcdir)/tools/xml2h.py $(srcdir)/plugins/usertools/python-tool-setup.py \
# 		plugins/usertools/python-tool-setup.h.tmp PYTHON_TOOL_SETUP_PY \
# 		&& mv plugins/usertools/python-tool-setup.h.tmp plugins/usertools/python-tool-setup.h
# endif

# cfgdir = $(MOO_DATA_DIR)
# cfg_DATA =					\
# 	plugins/usertools/filters.xml		\
# 	plugins/usertools/menu.xml		\
# 	plugins/usertools/context.xml
# EXTRA_DIST +=					\
# 	plugins/usertools/filters.xml		\
# 	plugins/usertools/menu-tmpl.xml		\
# 	plugins/usertools/context-tmpl.xml	\
# 	plugins/usertools/genplatxml.py
#
# genplatxml_args =
#
# if MOO_OS_WIN32
# genplatxml_args += --enable=win32
# else !MOO_OS_WIN32
# genplatxml_args += --enable=unix
# endif !MOO_OS_WIN32
#
# if MOO_ENABLE_PYTHON
# genplatxml_args += --enable=python
# endif
#
# built_plugins_sources += plugins/usertools/menu.xml plugins/usertools/context.xml
# plugins/usertools/%.xml: plugins/usertools/%-tmpl.xml plugins/usertools/genplatxml.py
# 	$(AM_V_at)$(MKDIR_P) plugins/usertools
# 	$(AM_V_GEN)$(MOO_PYTHON) $(srcdir)/plugins/usertools/genplatxml.py $(genplatxml_args) \
# 		$(srcdir)/plugins/usertools/$*-tmpl.xml > $@.tmp && mv $@.tmp $@
