# -%- lang: makefile; indent-width: 8; use-tabs: true -%-
# UGLY_MK_TRIGGER = QT_QRC_FILES

##################################################################################
#
# qrc
#
# input: QT_QRC_FILES - list of .qrc files
#        QT_QRC_DEPS
#

ugly_qrc_name = @MODULE@
ugly_qrc_cpp = @MODULE@-qrc.cpp
$(ugly_qrc_cpp): $(QT_QRC_FILES) $(QT_QRC_DEPS)
	$(QT_RCC) -o $(ugly_qrc_cpp).tmp -name $(ugly_qrc_name) \
	    $(addprefix $(srcdir)/,$(QT_QRC_FILES)) && mv $(ugly_qrc_cpp).tmp $(ugly_qrc_cpp)

EXTRA_DIST += $(QT_QRC_FILES) $(QT_QRC_DEPS)
BUILT_SOURCES += $(ugly_qrc_cpp)
@MODULE@_SOURCES += $(ugly_qrc_cpp)
