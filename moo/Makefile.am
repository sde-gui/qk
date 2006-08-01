moo_include_headers =	\
	moo.h		\
	moo-version.h

if MOO_INSTALL_LIB
moo_include_HEADERS = $(moo_include_headers)
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = moo.pc
endif

EXTRA_DIST =	\
	TODO

subdirs =
libs =
if MOO_BUILD_UTILS
subdirs += mooutils
libs += mooutils/libmooutils.la
endif
if GTK_2_6
subdirs += moofileview
libs += moofileview/libmoofileview.la
endif
if MOO_BUILD_TERM
subdirs += mooterm
libs += mooterm/libmooterm.la
endif
if MOO_BUILD_EDIT
subdirs += mooscript mooedit
libs += mooscript/libmooscript.la mooedit/libmooedit.la
endif
if MOO_BUILD_APP
subdirs += mooapp
libs += mooapp/libmooapp.la
endif
if MOO_USE_PYGTK
subdirs += moopython
libs += moopython/libmoopython.la
endif

SUBDIRS = $(subdirs) .
DIST_SUBDIRS = mooutils mooterm mooedit mooapp moopython mooscript moofileview

if MOO_INSTALL_LIB
lib_LTLIBRARIES = libmoo.la
else
noinst_LTLIBRARIES = libmoo.la
endif

#lib_LTLIBRARIES = libmoo.la
libmoo_la_LIBADD = $(libs) $(MOO_LIBS) $(PYTHON_LDFLAGS)
# libmoo_la_LDFLAGS = -no-undefined -export-symbols-regex "^(moo_|ms_|m_|egg_regex_).*"
libmoo_la_LDFLAGS = -no-undefined -export-symbols-regex ".*"
libmoo_la_SOURCES = $(moo_include_headers)


if MOO_INSTALL_LIB
if MOO_USE_PYGTK
pyexec_LTLIBRARIES = moo.la
moo_la_SOURCES =			\
	moopython/moopython-utils.c	\
	moopython/moopython-utils.h	\
	moopython/mooplugin-python.c	\
	moopython/mooplugin-python.h
moo_la_LDFLAGS = -module -no-undefined -export-symbols-regex "initmoo"
moo_la_LIBADD = moopython/pygtk/libmoopygtk.la libmoo.la
moo_la_CFLAGS =			\
	-I..			\
	-I$(top_builddir)	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)	\
	$(PYTHON_INCLUDES)	\
	$(PYGTK_CFLAGS)		\
	-DMOO_PYTHON_MODULE
endif
endif
