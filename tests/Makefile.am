AM_CFLAGS =			\
	-I$(top_builddir)	\
	-I../moo		\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)

noinst_PROGRAMS =
EXTRA_PROGRAMS =			\
	testhtml testtext medit mterm	\
	markup termbuffer testfileview	\
	testpaned testpanedfileview 	\
	miniglade langparser testobject \
	testfold ms testkeyfile

all-tests: $(EXTRA_PROGRAMS)

EXTRA_DIST =		\
	pyapp.py.in	\
	medit-ui.xml	\
	medit-app.opag	\
	medit.win

BUILT_SOURCES = medit-ui.h THANKS.h

XML2H = $(srcdir)/../moo/mooutils/xml2h.sh
medit-ui.h: medit-ui.xml $(XML2H)
	$(SHELL) $(XML2H) MEDIT_UI $(srcdir)/medit-ui.xml > medit-ui.h.tmp && \
	mv medit-ui.h.tmp medit-ui.h
THANKS.h: ../THANKS $(XML2H)
	$(SHELL) $(XML2H) THANKS $(srcdir)/../THANKS > THANKS.h

# medit-app.c is not in BUILT_SOURCES, so the target must use absolute path to it
$(srcdir)/medit-app.c: medit-app.opag medit-ui.h
	if test -z $(OPAG); then \
		touch $(srcdir)/medit-app.c; \
	else \
		$(OPAG) -f _medit_parse_options -O _medit_opt_ -A _medit_arg_ $(srcdir)/medit-app.opag $(srcdir)/medit-app.c; \
	fi

if MOO_OS_MINGW
medit.win: medit.win
	cp $(srcdir)/medit.win medit
	chmod +x medit
endif

all-am: pyapp.py

if MOO_BUILD_APP
medit = medit
endif
if MOO_BUILD_EDIT
medit = medit
# noinst_PROGRAMS += testtext
endif
if MOO_BUILD_TERM
if !MOO_OS_CYGWIN
if !MOO_BUILD_APP
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

bin_PROGRAMS = $(medit)

ldflags = -export-dynamic
ldadd = ../moo/libmoo.la
test_ldflags = -export-dynamic
test_ldadd = ../moo/libmoo-test.la

if MOO_OS_MINGW
ldflags += -mwindows
test_ldflags += -mwindows
endif MOO_OS_MINGW


##############################################################################
##  medit
##
medit_LDFLAGS = $(ldflags)
medit_LDADD = $(ldadd)
medit_SOURCES = medit-ui.h THANKS.h
nodist_medit_SOURCES =
EXTRA_DIST += medit.rc.in medit-app.c

if MOO_BUILD_APP
medit_SOURCES += medit-app.c
else
medit_SOURCES += medit.c
endif

nodist_medit_SOURCES += medit.res
CLEANFILES = medit.res

if MOO_OS_MINGW
medit.res: medit.rc
	$(WINDRES) -i medit.rc --input-format=rc -o medit.res -O coff
medit_SOURCES += medit.res
medit_LDADD += medit.res
else ! MOO_OS_MINGW
medit.res: medit.rc
	touch medit.res
endif ! MOO_OS_MINGW


testfold_LDFLAGS = $(ldflags)
testfold_LDADD = $(ldadd)
testfold_SOURCES = testfold.c


testtext_LDFLAGS = $(ldflags)
testtext_LDADD = $(ldadd)
testtext_SOURCES = testtext.c


langparser_LDFLAGS = $(ldflags)
langparser_LDADD = $(ldadd)
langparser_SOURCES = testparser.c

testkeyfile_LDFLAGS = $(ldflags)
testkeyfile_LDADD = $(ldadd)
testkeyfile_SOURCES = testkeyfile.c

ms_LDFLAGS = $(ldflags)
ms_LDADD = $(ldadd) -lreadline
ms_SOURCES = mscript.c

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
