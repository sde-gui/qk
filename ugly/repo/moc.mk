# -%- lang: makefile; indent-width: 8; use-tabs: true -%-
# UGLY_MK_TRIGGER = QT_MOC_HDRS

##################################################################################
#
# moc
#
# input: QT_MOC_HDRS - list of .h files to be moc'ed
#

ugly_moc_wrapper = $(top_srcdir)/ugly/moc-wrapper

all-classes-moc.cpp.stamp: $(QT_MOC_HDRS) Makefile $(ugly_moc_wrapper)
	$(ugly_moc_wrapper) $(QT_MOC) $(srcdir) all-classes-moc.cpp $(QT_MOC_HDRS) && echo stamp > $@

ugly_moc_sources = all-classes-moc.cpp
ugly_moc_stamps = all-classes-moc.cpp.stamp

EXTRA_DIST += $(QT_MOC_HDRS) $(ugly_moc_wrapper)
BUILT_SOURCES += $(ugly_moc_stamps)
CLEANFILES += $(ugly_moc_sources) $(ugly_moc_stamps)
nodist_@MODULE@_SOURCES += $(ugly_moc_sources)
