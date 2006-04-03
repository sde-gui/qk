MOO = ../moo
MOO_W = ..\moo
XML2H = $(MOO)/mooutils/xml2h.py

mooutils = $(MOO)/mooutils
mooutils_srcdir = $(MOO)/mooutils
tests = ../tests

GENERATED =                                 \
    config.h                                \
    $(mooutils)/pcre/pcre.h                 \
    $(mooutils)/moomarshals.h               \
    $(mooutils)/moomarshals.c               \
    $(mooutils)/mooaccelbutton-glade.h      \
    $(mooutils)/mooaccelprefs-glade.h       \
    $(MOO)/mooterm/mootermprefs-glade.h     \
    $(tests)/mterm-ui.h                     \
    $(tests)/medit-ui.h

all-before: $(GENERATED)

##############################################################
# Marshalers
#
$(MOO)/mooutils/moomarshals.h: $(MOO)/mooutils/moomarshals.list
	glib-genmarshal --prefix=_moo_marshal --header $(MOO)/mooutils/moomarshals.list > $(MOO)/mooutils/moomarshals.h

$(MOO)/mooutils/moomarshals.c: $(MOO)/mooutils/moomarshals.list
	glib-genmarshal --prefix=_moo_marshal --body $(MOO)/mooutils/moomarshals.list > $(MOO)/mooutils/moomarshals.c


##############################################################
# config.h
#
$(MOO)/mooutils/pcre/pcre.h: $(MOO)/mooutils/pcre/pcre.h.win32
	copy /Y $(MOO_W)\mooutils\pcre\pcre.h.win32 $(MOO_W)\mooutils\pcre\pcre.h
config.h: ../config.h.win32
	copy /Y ..\config.h.win32 config.h


##############################################################
# glade files
#
$(MOO)/mooterm/mootermprefs-glade.h: $(MOO)/mooterm/glade/mootermprefs.glade $(XML2H)
	python $(XML2H) MOO_TERM_PREFS_GLADE_UI $(MOO)/mooterm/glade/mootermprefs.glade \
		> $(MOO)/mooterm/mootermprefs-glade.h
$(mooutils)/mooaccelbutton-glade.h: $(mooutils_srcdir)/glade/shortcutdialog.glade $(XML2H)
	python $(XML2H) MOO_ACCEL_BUTTON_GLADE_UI \
	   $(mooutils_srcdir)/glade/shortcutdialog.glade > $(mooutils)/mooaccelbutton-glade.h
$(mooutils)/mooaccelprefs-glade.h: $(mooutils_srcdir)/glade/shortcutsprefs.glade $(XML2H)
	python $(XML2H) MOO_SHORTCUTS_PREFS_GLADE_UI \
	   $(mooutils_srcdir)/glade/shortcutsprefs.glade > $(mooutils)/mooaccelprefs-glade.h


##############################################################
# xml
#
$(tests)/medit-ui.h: $(tests)/medit-ui.xml $(XML2H)
	python $(XML2H) MEDIT_UI $(tests)/medit-ui.xml > $(tests)/medit-ui.h

$(tests)/mterm-ui.h: $(tests)/mterm-ui.xml $(XML2H)
	python $(XML2H) MTERM_UI $(tests)/mterm-ui.xml > $(tests)/mterm-ui.h
