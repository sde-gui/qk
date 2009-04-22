# -%- lang: makefile; indent-width: 8; use-tabs: true -%-
# UGLY_MODULE_TRIGGER = RC_IN_FILE
# UGLY_MODULE_VARS = LDFLAGS

# mod_$(RC_IN_FILE) should be a list of input *.rc.in files

if MOO_OS_MINGW

%.res: %.rc.in $(top_builddir)/config.status
	cd $(top_builddir) && \
	$(SHELL) ./config.status --file=$(subdir)/$*.rc && \
	cd $(subdir) && \
	$(WINDRES) -i $*.rc --input-format=rc -o $@ -O coff && \
	rm $*.rc

@MODULE@_ugly_res_file = $(@MODULE@_RC_IN_FILE:.rc.in=.res)

CLEANFILES += $(@MODULE@_ugly_res_file)
BUILT_SOURCES += $(@MODULE@_ugly_res_file)
@MODULE@_LDFLAGS += -Wl,$(@MODULE@_ugly_res_file)

endif

EXTRA_DIST += $(@MODULE@_RC_IN_FILE)
