moo_include_headers =	\
	moo.h		\
	moo-version.h

# moo_libinclude_headers =            \
#     $(moo_prefix)/moo-config.h

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

# noinst_LTLIBRARIES = libmoo.la
lib_LTLIBRARIES = libmoo.la
libmoo_la_LIBADD = $(libs)
# libmoo_la_LDFLAGS = -no-undefined -export-symbols-regex "^(moo_|ms_|m_|egg_regex_).*"
libmoo_la_LDFLAGS = -no-undefined -export-symbols-regex ".*"
libmoo_la_SOURCES = $(moo_include_headers)
