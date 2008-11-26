# -%- lang: sh; indent-width: 8; use-tabs: true -%-
# UGLY_MK_TRIGGER = QTESTS
# UGLY_MK_VARS = EXTRA_PROGRAMS
# UGLY_MK_TYPE = sh

##################################################################################
#
# qtest
#
# input: QTESTS - list of tests. There must be corresponding -t.h files which
# contain the testing classes.
#

MODULE=`echo $MODULE | sed 's/\./_/'`

cat <<\EOFEOF
TESTS =
ugly_qtest_cpp = $(top_srcdir)/ugly/qtest.cpp

if MOO_ENABLE_UNIT_TESTS

TESTS += $(QTESTS:=-test)
EXTRA_PROGRAMS += $(QTESTS:=-test)
BUILT_SOURCES += $(QTESTS:=-test.cpp) $(QTESTS:=-test.moc)
CLEANFILES += $(QTESTS:=-test.cpp) $(QTESTS:=-test.moc)

%-test.cpp: $(ugly_qtest_cpp)
	sed -e 's/@test@/$*/g' $(ugly_qtest_cpp) > $*-test.cpp.tmp && mv $*-test.cpp.tmp $*-test.cpp

%-test.moc: %-t.h
	$(ugly_moc_wrapper) $(QT_MOC) $(srcdir) $*-test.moc $*-t.h

EOFEOF

for name in $QTESTS; do
var=`echo $name | sed 's/-/_/g'`
cat <<EOFEOF
${var}_test_SOURCES = ${name}-t.h
nodist_${var}_test_SOURCES = ${name}-test.cpp
${var}_test_LDADD = \$(patsubst %.cpp,%.o,\$(filter %.cpp,\$(filter-out main.cpp,\$(${MODULE}_SOURCES) \$(nodist_${MODULE}_SOURCES)))) \$(qtest_ldadd)
EOFEOF
done

cat <<\EOFEOF

endif

EXTRA_DIST += $(QTESTS:=-t.h) $(ugly_qtest_cpp)
EOFEOF
