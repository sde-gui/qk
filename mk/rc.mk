# -*- makefile -*-

# $(rc_in_files) should be a list of input *.rc.in files
# $(rc_files) is defined to be a list of generated *.res files

if MOO_OS_MINGW

%.res: %.rc.in $(top_builddir)/config.status
	cd $(top_builddir) && \
	$(SHELL) ./config.status --file=$(subdir)/$*.rc && \
	cd $(subdir) && \
	$(WINDRES) -i $*.rc --input-format=rc -o $@ -O coff && \
	rm $*.rc

rc_files = $(rc_in_files:.rc.in=.res)

CLEANFILES += $(rc_files)
BUILT_SOURCES += $(rc_files)

endif

EXTRA_DIST += $(rc_in_files)
