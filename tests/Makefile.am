AM_CFLAGS = $(MOO_CFLAGS)
AM_CXXFLAGS = $(MOO_CXXFLAGS)

check_PROGRAMS =			\
	markup testpaned		\
	miniglade testobject		\
	testthreads testthreads2
TESTS =					\
	testthreads			\
	testthreads2.sh			\
	markup

if MOO_BUILD_EDIT
check_PROGRAMS +=			\
	testtext testfileview		\
	testpanedfileview testfold	\
	testkeyfile
TESTS +=				\
	testfold
endif
if MOO_BUILD_APP
check_PROGRAMS += testhtml
endif

EXTRA_DIST = testthreads2.sh
CLEANFILES = testthreads2-log.txt

all-tests: $(check_PROGRAMS)

if MOO_BUILD_UTILS
if !MOO_BUILD_EDIT
check_PROGRAMS += testfileview
endif
endif

ldflags = -export-dynamic $(PYTHON_EXTRA_LDFLAGS)
include ../moo/moo-comps.mk
ldadd = $(moo_app_ldadd)

testfold_LDFLAGS = $(ldflags)
testfold_LDADD = $(ldadd)
testfold_SOURCES = testfold.c

testtext_LDFLAGS = $(ldflags)
testtext_LDADD = $(ldadd)
testtext_SOURCES = testtext.c

testkeyfile_LDFLAGS = $(ldflags)
testkeyfile_LDADD = $(ldadd)
testkeyfile_SOURCES = testkeyfile.c

testthreads_LDFLAGS = $(ldflags)
testthreads_LDADD = $(ldadd) $(GTHREAD_LIBS)
testthreads_SOURCES = testthreads.c

testthreads2_LDFLAGS = $(ldflags)
testthreads2_LDADD = $(ldadd) $(GTHREAD_LIBS)
testthreads2_SOURCES = testthreads2.c

testhtml_LDFLAGS = $(ldflags)
testhtml_LDADD = $(ldadd)
testhtml_SOURCES = testhtml.c


##############################################################################
##  markup
##
markup_LDFLAGS = $(ldflags)
markup_LDADD = $(ldadd)

markup_SOURCES = markup.c


##############################################################################
##  miniglade
##
miniglade_LDFLAGS = $(ldflags)
miniglade_LDADD = $(ldadd)

miniglade_SOURCES = miniglade.c


##############################################################################

testpaned_LDFLAGS = $(ldflags)
testpaned_LDADD = $(ldadd)
testpaned_SOURCES = testpaned.c

testfileview_LDFLAGS = $(ldflags)
testfileview_LDADD = $(ldadd)
testfileview_SOURCES = testfileview.c

testpanedfileview_LDFLAGS = $(ldflags)
testpanedfileview_LDADD = $(ldadd)
testpanedfileview_SOURCES = testpanedfileview.c

testobject_LDFLAGS = $(ldflags)
testobject_LDADD = $(ldadd)
testobject_SOURCES = testobject.c
