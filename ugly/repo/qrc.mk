# -%- lang: makefile; indent-width: 8; use-tabs: true -%-
# UGLY_MK_TRIGGER = RESOURCES

##################################################################################
#
# qrc
#
# input: RESOURCES - list of .qrc files
#        QT_QRC_DEPS
#

ugly_qrc_name = @MODULE@
ugly_qrc_cpp = @MODULE@-qrc.cpp
$(ugly_qrc_cpp): $(RESOURCES) $(QT_QRC_DEPS)
	$(QT_RCC) -o $(ugly_qrc_cpp).tmp -name $(ugly_qrc_name) \
	    $(addprefix $(srcdir)/,$(RESOURCES)) && mv $(ugly_qrc_cpp).tmp $(ugly_qrc_cpp)

EXTRA_DIST += $(RESOURCES) $(QT_QRC_DEPS)
BUILT_SOURCES += $(ugly_qrc_cpp)
CLEANFILES += $(ugly_qrc_cpp)
nodist_@MODULE@_SOURCES += $(ugly_qrc_cpp)
