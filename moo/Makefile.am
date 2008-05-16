AM_CFLAGS =			\
	-I$(top_builddir)	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)

ini_in_files = moopython/pymoo.ini.in
ini_files = $(ini_in_files:.ini.in=.ini)
%.ini: %.ini.in $(top_builddir)/config.status
	cd $(top_builddir) && $(SHELL) ./config.status --file=$(subdir)/$@
CLEANFILES = $(ini_files)

moo-config-dirs.h: moo-config-dirs.h.stamp
moo-config-dirs.h.stamp: Makefile
	{ echo '#define MOO_DATA_DIR "$(MOO_DATA_DIR)"'; \
	  echo '#define MOO_LIB_DIR "$(MOO_LIB_DIR)"'; \
	  echo '#define MOO_LOCALE_DIR "$(MOO_LOCALE_DIR)"'; \
	  echo '#define MOO_HELP_DIR "$(MOO_HELP_DIR)"'; \
	} | sed '/""/d' > moo-config-dirs.h.tmp || exit 1
	cmp -s moo-config-dirs.h moo-config-dirs.h.tmp || \
	    mv moo-config-dirs.h.tmp moo-config-dirs.h || exit 1
	rm -f moo-config-dirs.h.tmp
	echo stamp > moo-config-dirs.h.stamp
BUILT_SOURCES = moo-config-dirs.h.stamp
CLEANFILES += moo-config-dirs.h.stamp moo-config-dirs.h

test_sources =			\
	moo-test-utils.c	\
	moo-test-utils.h	\
	moo-test-macros.h	\
	moo-tests-lua.h		\
	moo-tests.h

BUILT_SOURCES += moo-config.h
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
if MOO_BUILD_UI
subdirs += mooui
libs += mooui/libmooui.la
marshals += $(srcdir)/mooui/marshals.list
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

if MOO_USE_PYTHON
subdirs += moopython
endif

SUBDIRS = $(subdirs) .
DIST_SUBDIRS = moolua mooutils mooterm mooedit mooui moopython moofileview

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

rc_in_files = libmoo.rc.in moo.rc.in pymoo.rc.in
include ../mk/rc.mk

if MOO_OS_MINGW
CLEANFILES += libmoo.res
libmoo_res_ldflag = -Wl,libmoo.res
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
libmoo_la_SOURCES = moo.h
nodist_libmoo_la_SOURCES += moo-config.h

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
