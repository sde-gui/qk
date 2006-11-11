moo_include_headers =	\
	moo.h		\
	moo-version.h

if MOO_INSTALL_HEADERS
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
if !MOO_BUILD_PYTHON_MODULE
libs += moopython/libmoopython.la $(PYTHON_LIBS) $(PYTHON_EXTRA_LIBS)
endif
endif

SUBDIRS = $(subdirs) .
DIST_SUBDIRS = mooutils mooterm mooedit mooapp moopython mooscript moofileview

lib_LTLIBRARIES =
noinst_LTLIBRARIES =

if MOO_INSTALL_LIB
lib_LTLIBRARIES += libmoo.la
else
noinst_LTLIBRARIES += libmoo.la
endif

libmoo_la_LIBADD = $(libs) $(MOO_LIBS)
# libmoo_la_LDFLAGS = $(PYTHON_EXTRA_LDFLAGS) -no-undefined -avoid-version -export-symbols-regex "^(moo_|ms_).*"
libmoo_la_LDFLAGS = $(PYTHON_EXTRA_LDFLAGS) -no-undefined -avoid-version -export-symbols-regex ".*"
libmoo_la_SOURCES = $(moo_include_headers)

moo_la_ldflags = -no-undefined -module -avoid-version -export-symbols-regex initmoo
pymoo_la_ldflags = -no-undefined -module -avoid-version -export-symbols-regex moo_module_init

if MOO_OS_UNIX

if MOO_INSTALL_LIB

if MOO_USE_PYGTK
pyexec_LTLIBRARIES = moo.la
moo_la_SOURCES = moo.h
moo_la_LDFLAGS = $(moo_la_ldflags)
moo_la_LIBADD = moopython/libmoomod.la libmoo.la $(MOO_LIBS) $(PYTHON_LIBS)
endif MOO_USE_PYGTK

if MOO_BUILD_PYTHON_MODULE
plugindir = $(MOO_LIB_DIR)/plugins
plugin_LTLIBRARIES = pymoo.la
plugin_DATA = moopython/pymoo.ini
pymoo_la_SOURCES = moo.h
pymoo_la_LDFLAGS = $(pymoo_la_ldflags) $(PYTHON_EXTRA_LDFLAGS)
pymoo_la_LIBADD = moopython/libpymoo.la libmoo.la $(MOO_LIBS) $(PYTHON_LIBS) $(PYTHON_EXTRA_LIBS)
endif MOO_BUILD_PYTHON_MODULE

endif MOO_INSTALL_LIB

else !MOO_OS_UNIX
if MOO_USE_PYGTK

lib_LTLIBRARIES += pymoo25.la moo25.la pymoo24.la moo24.la
plugindir = $(MOO_LIB_DIR)/plugins
plugin_DATA = moopython/pymoo25.ini moopython/pymoo24.ini

pymoo25_la_SOURCES = moo.h
pymoo25_la_LDFLAGS = $(pymoo_la_ldflags) $(PYTHON25_EXTRA_LDFLAGS)
pymoo25_la_LIBADD = moopython/libpymoo25.la libmoo.la $(MOO_LIBS) $(PYTHON25_LIBS) $(PYTHON25_EXTRA_LIBS)
moo25_la_SOURCES = moo.h
moo25_la_LDFLAGS = $(moo_la_ldflags)
moo25_la_LIBADD = moopython/libmoomod25.la libmoo.la $(MOO_LIBS) $(PYTHON25_LIBS)

pymoo24_la_SOURCES = moo.h
pymoo24_la_LDFLAGS = $(pymoo_la_ldflags) $(PYTHON24_EXTRA_LDFLAGS)
pymoo24_la_LIBADD = moopython/libpymoo24.la libmoo.la $(MOO_LIBS) $(PYTHON24_LIBS) $(PYTHON24_EXTRA_LIBS)
moo24_la_SOURCES = moo.h
moo24_la_LDFLAGS = $(moo_la_ldflags)
moo24_la_LIBADD = moopython/libmoomod24.la libmoo.la $(MOO_LIBS) $(PYTHON24_LIBS)

endif MOO_USE_PYGTK
endif !MOO_OS_UNIX


if MOO_ENABLE_TESTS
noinst_LTLIBRARIES += libmoo-test.la
endif

libmoo_test_la_LIBADD = $(libmoo_la_LIBADD)
libmoo_test_la_LDFLAGS = $(PYTHON_EXTRA_LDFLAGS) -no-undefined -avoid-version
libmoo_test_la_SOURCES = $(moo_include_headers)
