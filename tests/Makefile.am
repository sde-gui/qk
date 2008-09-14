AM_CFLAGS = $(MOO_CFLAGS)
AM_CXXFLAGS = $(MOO_CXXFLAGS)

noinst_PROGRAMS =

test_programs =				\
	markup testpaned miniglade	\
	testobject testthreads

if MOO_BUILD_EDIT
test_programs +=			\
	testtext testfileview		\
	testpanedfileview testfold	\
	testkeyfile
endif
if MOO_BUILD_APP
test_programs += testhtml
endif

EXTRA_PROGRAMS = $(test_programs)

if MOO_ENABLE_TESTS
noinst_PROGRAMS += $(test_programs)
endif

all-tests: $(EXTRA_PROGRAMS)

EXTRA_DIST =		\
	pyapp.py.in

BUILT_SOURCES = pyapp.py

if MOO_BUILD_UTILS
if !MOO_BUILD_EDIT
noinst_PROGRAMS += testfileview
endif
endif

ldflags = -export-dynamic $(PYTHON_EXTRA_LDFLAGS)
include ../moo/moo-comps.mk
ldadd = $(moo_app_ldadd)

# if MOO_OS_MINGW
# ldflags += -mwindows
# endif MOO_OS_MINGW

run_tests_LDFLAGS = $(ldflags)
run_tests_LDADD = $(ldadd)


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
