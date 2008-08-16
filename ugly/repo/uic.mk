# -*- makefile -*-
# UGLY_MK_TRIGGER = QT_UI_FILES

##################################################################################
#
# uic
#
# input: QT_UI_FILES - list of .ui files
#

ugly_uic_wrapper = $(top_srcdir)/ugly/uic-wrapper
ugly_ui_names = $(patsubst %.ui,%,$(QT_UI_FILES))
ui.h.stamp: $(QT_UI_FILES) Makefile $(ugly_uic_wrapper)
	$(ugly_uic_wrapper) $(QT_UIC) $(srcdir) $(ugly_ui_names) && echo stamp > ui.h.stamp

EXTRA_DIST += $(QT_UI_FILES) $(ugly_uic_wrapper)
BUILT_SOURCES += ui.h.stamp
ugly_ui_headers = $(patsubst %.ui,ui_%.h,$(QT_UI_FILES))
CLEANFILES += $(ugly_ui_headers) ui.h.stamp
nodist_@MODULE@_SOURCES += $(ugly_ui_headers)
