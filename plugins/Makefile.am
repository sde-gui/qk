SUBDIRS = script usertools

noinst_LTLIBRARIES = libmooeditplugins.la

libmooeditplugins_la_LIBADD = script/libmomscript.la usertools/libmoousertools.la

AM_CPPFLAGS = $(MOO_CPPFLAGS) -I$(top_srcdir)/moo -I$(top_srcdir)/doc -I../moo
AM_CFLAGS = $(MOO_CFLAGS)
AM_CXXFLAGS = $(MOO_CXXFLAGS)

EXTRA_DIST =
BUILT_SOURCES =
CLEANFILES =

libmooeditplugins_la_SOURCES =	\
	moofileselector-prefs.c	\
	moofileselector.c	\
	moofileselector.h	\
	mooeditplugins.h	\
	mooeditplugins.c	\
	moofilelist.c		\
	moofind.c

EXTRA_DIST +=					\
	glade/moofileselector-prefs.glade	\
	glade/moofileselector.glade		\
	glade/moofind.glade			\
	glade/moogrep.glade

BUILT_SOURCES +=				\
	glade/moofileselector-prefs-gxml.h	\
	glade/moofileselector-gxml.h		\
	glade/moofind-gxml.h			\
	glade/moogrep-gxml.h

CLEANFILES +=					\
	glade/moofileselector-prefs-gxml.h	\
	glade/moofileselector-gxml.h		\
	glade/moofind-gxml.h			\
	glade/moogrep-gxml.h

include ctags/Makefile.incl

include ../tools/rules.mk
