rc_in_files = libmoo.rc.in moo24.rc.in moo25.rc.in pymoo24.rc.in pymoo25.rc.in
ini_in_files = moopython/pymoo.ini.in moopython/pymoo24.ini.in moopython/pymoo25.ini.in
ini_files = $(ini_in_files:.ini.in=.ini)
@MOO_INI_IN_RULE@
CLEANFILES = $(ini_files)

moo_include_headers =		\
	moo.h			\
	moo-config.h

BUILT_SOURCES = moo-config.h

scriptsdir = ${MOO_DATA_DIR}/scripts
scripts_SCRIPTS =		\
	xdg-utils/xdg-open	\
	xdg-utils/xdg-email

EXTRA_DIST =			\
	$(rc_in_files)		\
	$(ini_in_files)		\
	$(scripts_SCRIPTS)

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
if MOO_BUILD_SCRIPT
subdirs += mooscript
libs += mooscript/libmooscript.la
endif
if MOO_BUILD_EDIT
subdirs += mooedit
libs += mooedit/libmooedit.la
endif
if MOO_BUILD_APP
subdirs += mooapp
libs += mooapp/libmooapp.la
endif

if MOO_USE_PYTHON
subdirs += moopython
endif

SUBDIRS = $(subdirs) .
DIST_SUBDIRS = mooutils mooterm mooedit mooapp moopython mooscript moofileview

lib_LTLIBRARIES =
noinst_LTLIBRARIES =

############################################################################
# libmoo
#
if !MOO_OS_CYGWIN
if MOO_INSTALL_LIB
lib_LTLIBRARIES += libmoo.la
else
noinst_LTLIBRARIES += libmoo.la
endif
endif

if MOO_OS_MINGW
@MOO_WIN32_RC_RULE@
CLEANFILES += libmoo.res
libmoo_res_ldflag = -Wl,libmoo.res
BUILT_SOURCES += libmoo.res
nodist_libmoo_la_SOURCES = libmoo.res
libs += -lwsock32
endif

if MOO_ENABLE_TESTS
export_flags = -export-symbols-regex ".*"
else
if MOO_DEBUG # && !MOO_ENABLE_TESTS
export_flags = -export-symbols-regex ".*"
else # !MOO_DEBUG && !MOO_ENABLE_TESTS
export_flags = -export-symbols-regex "^(moo_).*"
endif
endif

libmoo_la_LIBADD = $(libs) $(MOO_LIBS)
libmoo_la_LDFLAGS = $(libmoo_res_ldflag) -no-undefined -avoid-version $(export_flags)
libmoo_la_SOURCES = $(moo_include_headers)

if MOO_PYTHON_BUILTIN
if !MOO_BUILD_MOO_MODULE
libmoo_la_LIBADD += $(PYTHON_LIBS) $(PYTHON_EXTRA_LIBS) moopython/libmoopython.la
libmoo_la_LDFLAGS += $(PYTHON_LDFLAGS) $(PYTHON_EXTRA_LDFLAGS)
endif !MOO_BUILD_MOO_MODULE
endif


############################################################################
# python stuff
#
if MOO_USE_PYTHON

moo_la_ldflags = -no-undefined -module -avoid-version -export-symbols-regex initmoo
pymoo_la_ldflags = -no-undefined -module -avoid-version -export-symbols-regex moo_module_init

############################################################################
# Unix
#
if MOO_OS_UNIX

if MOO_BUILD_MOO_MODULE
pyexec_LTLIBRARIES = moo.la
moo_la_SOURCES = moo.h
moo_la_LDFLAGS = $(moo_la_ldflags) $(PYTHON_LDFLAGS)
moo_la_LIBADD = moopython/libmoomod.la libmoo.la $(MOO_LIBS) $(PYTHON_LIBS)
endif MOO_BUILD_MOO_MODULE

if MOO_BUILD_PYTHON_PLUGIN
plugindir = $(MOO_LIB_DIR)/plugins
plugin_LTLIBRARIES = pymoo.la
plugin_DATA = moopython/pymoo.ini
pymoo_la_SOURCES = moo.h
pymoo_la_LDFLAGS = $(pymoo_la_ldflags) $(PYTHON_LDFLAGS) $(PYTHON_EXTRA_LDFLAGS)
pymoo_la_LIBADD = moopython/libpymoo.la libmoo.la $(MOO_LIBS) $(PYTHON_LIBS) $(PYTHON_EXTRA_LIBS)
endif MOO_BUILD_PYTHON_PLUGIN

############################################################################
# Win32
#
else !MOO_OS_UNIX

CLEANFILES += pymoo25.res moo25.res pymoo24.res moo24.res
BUILT_SOURCES += pymoo25.res moo25.res pymoo24.res moo24.res
nodist_pymoo25_la_SOURCES = pymoo25.res
nodist_moo25_la_SOURCES = moo25.res
nodist_pymoo24_la_SOURCES = pymoo24.res
nodist_moo24_la_SOURCES = moo24.res

moo_la_ldflags += -shrext ".pyd"
lib_LTLIBRARIES += pymoo25.la moo25.la pymoo24.la moo24.la
plugindir = $(MOO_LIB_DIR)/plugins
plugin_DATA = moopython/pymoo25.ini moopython/pymoo24.ini

pymoo25_la_SOURCES = moo.h
pymoo25_la_LDFLAGS = $(pymoo_la_ldflags) $(PYTHON25_LDFLAGS) $(PYTHON25_EXTRA_LDFLAGS) -Wl,pymoo25.res
pymoo25_la_LIBADD = moopython/libpymoo25.la libmoo.la $(MOO_LIBS) $(PYTHON25_LIBS) $(PYTHON25_EXTRA_LIBS)
moo25_la_SOURCES = moo.h
moo25_la_LDFLAGS = $(moo_la_ldflags) $(PYTHON25_LDFLAGS) -Wl,moo25.res
moo25_la_LIBADD = moopython/libmoomod25.la libmoo.la $(MOO_LIBS) $(PYTHON25_LIBS)

pymoo24_la_SOURCES = moo.h
pymoo24_la_LDFLAGS = $(pymoo_la_ldflags) $(PYTHON24_LDFLAGS) $(PYTHON24_EXTRA_LDFLAGS) -Wl,pymoo24.res
pymoo24_la_LIBADD = moopython/libpymoo24.la libmoo.la $(MOO_LIBS) $(PYTHON24_LIBS) $(PYTHON24_EXTRA_LIBS)
moo24_la_SOURCES = moo.h
moo24_la_LDFLAGS = $(moo_la_ldflags) $(PYTHON24_LDFLAGS) -Wl,moo24.res
moo24_la_LIBADD = moopython/libmoomod24.la libmoo.la $(MOO_LIBS) $(PYTHON24_LIBS)

endif !MOO_OS_UNIX

endif MOO_USE_PYTHON
