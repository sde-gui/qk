# -%- lang: makefile; indent-width: 8; use-tabs: true -%-
# UGLY_MK_TRIGGER = FORMS

##################################################################################
#
# uic
#
# input: FORMS - list of .ui files
#

ugly_uic_wrapper = $(top_srcdir)/ugly/uic-wrapper
ugly_ui_names = $(patsubst %.ui,%,$(FORMS))
ui.h.stamp: $(FORMS) Makefile $(ugly_uic_wrapper)
	$(ugly_uic_wrapper) $(QT_UIC) $(srcdir) $(ugly_ui_names) && echo stamp > ui.h.stamp

EXTRA_DIST += $(FORMS) $(ugly_uic_wrapper)
BUILT_SOURCES += ui.h.stamp
ugly_ui_headers = $(patsubst %.ui,ui_%.h,$(FORMS))
CLEANFILES += $(ugly_ui_headers) ui.h.stamp
nodist_@MODULE@_SOURCES += $(ugly_ui_headers)
