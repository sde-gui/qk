rc_in_files = libmoo.rc.in moo24.rc.in moo25.rc.in pymoo24.rc.in pymoo25.rc.in
ini_in_files = moopython/pymoo.ini.in moopython/pymoo24.ini.in moopython/pymoo25.ini.in
ini_files = $(ini_in_files:.ini.in=.ini)
@MOO_INI_IN_RULE@
CLEANFILES = $(ini_files)

EXTRA_DIST =		\
	TODO		\
	$(rc_in_files)	\
	$(ini_in_files)

moo_include_headers =	\
	moo.h		\
	moo-version.h

if MOO_INSTALL_HEADERS
moo_include_HEADERS = $(moo_include_headers)
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = moo.pc
endif

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
if !MOO_BUILD_PYTHON_PLUGIN
if !MOO_BUILD_MOO_MODULE
libs += moopython/libmoopython.la $(PYTHON_LIBS) $(PYTHON_EXTRA_LIBS)
endif
endif
endif

SUBDIRS = $(subdirs) .
DIST_SUBDIRS = mooutils mooterm mooedit mooapp moopython mooscript moofileview

lib_LTLIBRARIES =
noinst_LTLIBRARIES =
BUILT_SOURCES =

if MOO_INSTALL_LIB
lib_LTLIBRARIES += libmoo.la
else
noinst_LTLIBRARIES += libmoo.la
endif

if MOO_OS_MINGW
@MOO_WIN32_RC_RULE@
CLEANFILES += libmoo.res
libmoo_res_ldflag = -Wl,libmoo.res
BUILT_SOURCES += libmoo.res
nodist_libmoo_la_SOURCES = libmoo.res
endif

if MOO_ENABLE_TESTS
export_flags = -export-symbols-regex ".*"
else
export_flags = -export-symbols-regex "^(moo_).*"
endif

libmoo_la_LIBADD = $(libs) $(MOO_LIBS)
libmoo_la_LDFLAGS = $(PYTHON_EXTRA_LDFLAGS) $(libmoo_res_ldflag) -no-undefined -avoid-version $(export_flags)
libmoo_la_SOURCES = $(moo_include_headers)

moo_la_ldflags = -no-undefined -module -avoid-version -export-symbols-regex initmoo
pymoo_la_ldflags = -no-undefined -module -avoid-version -export-symbols-regex moo_module_init

if MOO_OS_UNIX

if MOO_USE_PYGTK
if MOO_BUILD_MOO_MODULE
pyexec_LTLIBRARIES = moo.la
moo_la_SOURCES = moo.h
moo_la_LDFLAGS = $(moo_la_ldflags)
moo_la_LIBADD = moopython/libmoomod.la libmoo.la $(MOO_LIBS) $(PYTHON_LIBS)
endif MOO_BUILD_MOO_MODULE
endif MOO_USE_PYGTK

if MOO_BUILD_PYTHON_PLUGIN
plugindir = $(MOO_LIB_DIR)/plugins
plugin_LTLIBRARIES = pymoo.la
plugin_DATA = moopython/pymoo.ini
pymoo_la_SOURCES = moo.h
pymoo_la_LDFLAGS = $(pymoo_la_ldflags) $(PYTHON_EXTRA_LDFLAGS)
pymoo_la_LIBADD = moopython/libpymoo.la libmoo.la $(MOO_LIBS) $(PYTHON_LIBS) $(PYTHON_EXTRA_LIBS)
endif MOO_BUILD_PYTHON_PLUGIN

else !MOO_OS_UNIX
if MOO_USE_PYGTK

if MOO_OS_MINGW
CLEANFILES += pymoo25.res moo25.res pymoo24.res moo24.res
BUILT_SOURCES += pymoo25.res moo25.res pymoo24.res moo24.res
nodist_pymoo25_la_SOURCES = pymoo25.res
nodist_moo25_la_SOURCES = moo25.res
nodist_pymoo24_la_SOURCES = pymoo24.res
nodist_moo24_la_SOURCES = moo24.res
endif

lib_LTLIBRARIES += pymoo25.la moo25.la pymoo24.la moo24.la
plugindir = $(MOO_LIB_DIR)/plugins
plugin_DATA = moopython/pymoo25.ini moopython/pymoo24.ini

pymoo25_la_SOURCES = moo.h
pymoo25_la_LDFLAGS = $(pymoo_la_ldflags) $(PYTHON25_EXTRA_LDFLAGS) -Wl,pymoo25.res
pymoo25_la_LIBADD = moopython/libpymoo25.la libmoo.la $(MOO_LIBS) $(PYTHON25_LIBS) $(PYTHON25_EXTRA_LIBS)
moo25_la_SOURCES = moo.h
moo25_la_LDFLAGS = $(moo_la_ldflags) -Wl,moo25.res
moo25_la_LIBADD = moopython/libmoomod25.la libmoo.la $(MOO_LIBS) $(PYTHON25_LIBS)

pymoo24_la_SOURCES = moo.h
pymoo24_la_LDFLAGS = $(pymoo_la_ldflags) $(PYTHON24_EXTRA_LDFLAGS) -Wl,pymoo24.res
pymoo24_la_LIBADD = moopython/libpymoo24.la libmoo.la $(MOO_LIBS) $(PYTHON24_LIBS) $(PYTHON24_EXTRA_LIBS)
moo24_la_SOURCES = moo.h
moo24_la_LDFLAGS = $(moo_la_ldflags) -Wl,moo24.res
moo24_la_LIBADD = moopython/libmoomod24.la libmoo.la $(MOO_LIBS) $(PYTHON24_LIBS)

endif MOO_USE_PYGTK
endif !MOO_OS_UNIX
