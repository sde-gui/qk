noinst_LTLIBRARIES = libmoousertools.la

AM_CPPFLAGS = $(MOO_CPPFLAGS) -I$(top_srcdir)/moo -I$(srcdir)/.. -I$(top_srcdir)/doc
AM_CFLAGS = $(MOO_CFLAGS)
AM_CXXFLAGS = $(MOO_CXXFLAGS)

EXTRA_DIST =
BUILT_SOURCES =

libmoousertools_la_SOURCES =	\
	moousertools.c		\
	moousertools.h		\
	moousertools-prefs.c	\
	moousertools-prefs.h	\
	moocommand.c		\
	moocommand.h		\
	moocommand-private.h	\
	moocommanddisplay.c	\
	moocommanddisplay.h	\
	moooutputfilterregex.c	\
	moooutputfilterregex.h	\
	mookeyfile.c		\
	mookeyfile.h		\
	moousertools-enums.h	\
	moousertools-enums.c

include lua/Makefile.incl
include exe/Makefile.incl

EXTRA_DIST += glade/moousertools.glade
BUILT_SOURCES += glade/moousertools-gxml.h

include ../../tools/rules.mk

cfgdir = $(MOO_DATA_DIR)
cfg_DATA = filters.xml
EXTRA_DIST +=			\
	filters.xml		\
	win32/menu.cfg		\
	win32/context.cfg	\
	unix/menu.cfg		\
	unix/context.cfg

if MOO_OS_WIN32
cfg_DATA += win32/menu.cfg win32/context.cfg
else !MOO_OS_WIN32
cfg_DATA += unix/menu.cfg unix/context.cfg
endif !MOO_OS_WIN32
