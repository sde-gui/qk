AM_CFLAGS =			\
	-I$(top_builddir)	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)

rc_in_files = libmoo.rc.in moo.rc.in pymoo.rc.in
ini_in_files = moopython/pymoo.ini.in
ini_files = $(ini_in_files:.ini.in=.ini)
@MOO_INI_IN_RULE@
CLEANFILES = $(ini_files)

moo_include_headers =		\
	moo.h
nodist_moo_include_headers =	\
	moo-config.h

test_sources =			\
	moo-test-utils.c	\
	moo-test-utils.h	\
	moo-test-macros.h	\
	moo-tests-lua.h		\
	moo-tests.h

BUILT_SOURCES = moo-config.h
CLEANFILES += moo-config.h

scriptsdir = ${MOO_DATA_DIR}/scripts
scripts_SCRIPTS =		\
	xdg-utils/xdg-open	\
	xdg-utils/xdg-email	\
	moo-open-html-help

EXTRA_DIST =			\
	$(rc_in_files)		\
	$(ini_in_files)		\
	$(scripts_SCRIPTS)	\
	$(test_sources)

marshals =

if MOO_INSTALL_HEADERS
moo_include_HEADERS = $(moo_include_headers)
nodist_moo_include_HEADERS = $(nodist_moo_include_headers)
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = moo.pc
endif

subdirs =
libs =
if MOO_BUILD_LUA
subdirs += moolua
libs += moolua/libmoolua.la
endif
if MOO_BUILD_UTILS
subdirs += mooutils
libs += mooutils/libmooutils.la
marshals += $(srcdir)/mooutils/marshals.list
endif
if GTK_2_6
subdirs += moofileview
libs += moofileview/libmoofileview.la
marshals += $(srcdir)/moofileview/marshals.list
endif
if MOO_BUILD_TERM
subdirs += mooterm
libs += mooterm/libmooterm.la
marshals += $(srcdir)/mooterm/marshals.list
endif
if MOO_BUILD_EDIT
subdirs += mooedit
libs += mooedit/libmooedit.la
marshals += $(srcdir)/mooedit/marshals.list
endif
if MOO_BUILD_APP
subdirs += mooapp
libs += mooapp/libmooapp.la
marshals += $(srcdir)/mooapp/marshals.list
endif

if MOO_USE_PYTHON
subdirs += moopython
endif

SUBDIRS = $(subdirs) .
DIST_SUBDIRS = moolua mooutils mooterm mooedit mooapp moopython moofileview

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

nodist_libmoo_la_SOURCES =

BUILT_SOURCES += marshals.c marshals.list
nodist_libmoo_la_SOURCES += marshals.c
CLEANFILES += marshals.c marshals.list
marshals.c: marshals.list
	glib-genmarshal --prefix=_moo_marshal --header marshals.list > $@.tmp && \
	glib-genmarshal --prefix=_moo_marshal --body marshals.list >> $@.tmp && \
	mv $@.tmp $@
marshals.list: $(marshals)
	( cat $(marshals) | uniq ) > marshals.list.tmp && mv marshals.list.tmp marshals.list

if MOO_OS_MINGW
@MOO_WIN32_RC_RULE@
CLEANFILES += libmoo.res
libmoo_res_ldflag = -Wl,libmoo.res
BUILT_SOURCES += libmoo.res
CLEANFILES += libmoo.res
nodist_libmoo_la_SOURCES += libmoo.res
libs += -lwsock32
endif

if MOO_ENABLE_TESTS
export_flags = -export-symbols-regex ".*"
else
if MOO_DEBUG_ENABLED # && !MOO_ENABLE_TESTS
export_flags = -export-symbols-regex ".*"
else # !MOO_DEBUG_ENABLED && !MOO_ENABLE_TESTS
export_flags = -export-symbols-regex "^(moo_).*"
endif
endif

libmoo_la_LIBADD = $(libs) $(MOO_LIBS)
libmoo_la_LDFLAGS = $(libmoo_res_ldflag) -no-undefined -avoid-version $(export_flags)
libmoo_la_SOURCES = $(moo_include_headers)
nodist_libmoo_la_SOURCES += $(nodist_moo_include_headers)

if MOO_ENABLE_UNIT_TESTS
libmoo_la_SOURCES += $(test_sources)
endif

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

CLEANFILES += pymoo.res moo.res
BUILT_SOURCES += pymoo.res moo.res
nodist_pymoo_la_SOURCES = pymoo.res
nodist_moo_la_SOURCES = moo.res

moo_la_ldflags += -shrext ".pyd"
plugindir = $(MOO_LIB_DIR)/plugins
pluginlibdir = $(plugindir)/lib
plugin_LTLIBRARIES = pymoo.la
pluginlib_LTLIBRARIES = moo.la

plugin_DATA = moopython/pymoo.ini

install-data-hook:
	rm -f $(DESTDIR)$(plugindir)/pymoo.la
	rm -f $(DESTDIR)$(plugindir)/pymoo.dll.a
	rm -f $(DESTDIR)$(pluginlibdir)/moo.la
	rm -f $(DESTDIR)$(pluginlibdir)/moo.dll.a

pymoo_la_SOURCES = moo.h
pymoo_la_LDFLAGS = $(pymoo_la_ldflags) $(PYTHON_LDFLAGS) $(PYTHON_EXTRA_LDFLAGS) -Wl,pymoo.res
pymoo_la_LIBADD = moopython/libpymoo.la libmoo.la $(MOO_LIBS) $(PYTHON_LIBS) $(PYTHON_EXTRA_LIBS)
moo_la_SOURCES = moo.h
moo_la_LDFLAGS = $(moo_la_ldflags) $(PYTHON_LDFLAGS) -Wl,moo.res
moo_la_LIBADD = moopython/libmoomod.la libmoo.la $(MOO_LIBS) $(PYTHON_LIBS)

endif !MOO_OS_UNIX

endif MOO_USE_PYTHON
