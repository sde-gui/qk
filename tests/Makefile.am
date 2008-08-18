EXTRA_DIST =			\
	data/gobject.lua	\
	data/textview.lua	\
	data/mooedit.lua	\
	data/testustring.lua	\
	data/testunicode.lua	\
	data/lua/munit.lua

if MOO_ENABLE_UNIT_TESTS
TESTS = run-tests
endif

AM_CFLAGS = $(MOO_CFLAGS)
AM_CXXFLAGS = $(MOO_CXXFLAGS)

if MOO_ENABLE_UNIT_TESTS
AM_CFLAGS += -DSRCDIR=\"$(shell cd $(srcdir) && pwd)\" -DTOP_SRCDIR=\"$(shell cd $(top_srcdir) && pwd)\"
AM_CXXFLAGS += -DSRCDIR=\"$(shell cd $(srcdir) && pwd)\" -DTOP_SRCDIR=\"$(shell cd $(top_srcdir) && pwd)\"
endif

noinst_PROGRAMS =

unit_test_programs = run-tests

test_programs =				\
	markup testpaned miniglade	\
	testobject testthreads

if MOO_BUILD_EDIT
test_programs +=			\
	testtext testfileview		\
	testpanedfileview testfold	\
	testkeyfile
endif
if MOO_BUILD_TERM
test_programs += mterm termbuffer
endif
if MOO_BUILD_UI
test_programs += testhtml
endif

EXTRA_PROGRAMS = $(test_programs) $(unit_test_programs)

if MOO_ENABLE_TESTS
noinst_PROGRAMS += $(test_programs)
endif
if MOO_ENABLE_UNIT_TESTS
noinst_PROGRAMS += $(unit_test_programs)
endif

all-tests: $(EXTRA_PROGRAMS)

EXTRA_DIST +=		\
	pyapp.py.in

BUILT_SOURCES = pyapp.py

if MOO_BUILD_TERM
if !MOO_OS_CYGWIN
if !MOO_BUILD_UI
noinst_PROGRAMS += mterm #termbuffer
endif
endif
endif

if MOO_BUILD_UTILS
if !MOO_BUILD_EDIT
if !MOO_BUILD_TERM
if !MOO_OS_CYGWIN
noinst_PROGRAMS += testfileview
endif
endif
endif
endif

ldflags = -export-dynamic $(PYTHON_EXTRA_LDFLAGS)
ldadd = ../moo/libmoo.la

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


mterm_LDFLAGS = $(ldflags)
mterm_LDADD = $(ldadd)
mterm_SOURCES = mterm.c

termbuffer_LDFLAGS = $(ldflags)
termbuffer_LDADD = $(ldadd)
termbuffer_SOURCES = termbuffer.c


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
