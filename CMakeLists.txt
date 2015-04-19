cmake_minimum_required(VERSION 2.8)

list(APPEND EXTRA_DIST
    tools/genenums.py
    tools/glade2c.py
    tools/xml2h.py
    plat/win32/installer.iss.in
    plat/win32/istrans.sh
    plat/win32/mingw-configure
)

# zzz
# if MOO_OS_WIN32
# plat/win32/installer.iss: $(top_srcdir)/plat/win32/installer.iss.in $(top_builddir)/config.status
# 	$(AM_V_at)$(MKDIR_P) plat/win32
# 	$(AM_V_GEN)cd $(top_builddir) && ./config.status --silent --file=plat/win32/installer.iss
# installer: plat/win32/installer.iss install
# 	$(MEDIT_INNO_COMPILER) plat/win32/installer.iss
# copy:
# 	cp moo/medit.exe "/win/Program Files (x86)/medit/bin/"
# bd_tmpdir = medit-win-dist-tmp
# bd_distdir = $(PACKAGE)-$(VERSION)
# bd_pzip = $(PACKAGE)-portable-$(VERSION)
# bd_zip = $(PACKAGE)-$(VERSION)
# portable: install
# 	rm -fr $(bd_tmpdir)
# 	mkdir $(bd_tmpdir)
# 	cp -lR ${prefix} $(bd_tmpdir)/$(bd_distdir)
# 	cd $(bd_tmpdir) && zip -r9 $(bd_zip).zip $(bd_distdir)
# 	echo "This file enables portable mode for medit" > $(bd_tmpdir)/$(bd_distdir)/bin/$(MEDIT_PORTABLE_MAGIC_FILE_NAME)
# 	cd $(bd_tmpdir) && zip -r9 $(bd_pzip).zip $(bd_distdir)
# 	mv $(bd_tmpdir)/$(bd_pzip).zip $(bd_tmpdir)/$(bd_zip).zip .
# 	rm -fr $(bd_tmpdir)
# bdist: installer portable
# endif

# zzz @MOO_PO_SUBDIRS_RULE@

project(medit)

# keep in sync with po/maintain
SET(MOO_PACKAGE_NAME "medit-1")

SET(MOO_MAJOR_VERSION 1)
SET(MOO_MINOR_VERSION 2)
SET(MOO_MICRO_VERSION 90)
SET(_moo_version_suffix_ "-devel")
SET(MOO_MODULE_MAJOR_VERSION 2)
SET(MOO_MODULE_MINOR_VERSION 0)
SET(MOO_VERSION "${MOO_MAJOR_VERSION}.${MOO_MINOR_VERSION}.${MOO_MICRO_VERSION}")
SET(MOO_DISPLAY_VERSION "${MOO_VERSION}${_moo_version_suffix_}")

SET(MOO_WEBSITE "http://mooedit.sourceforge.net/")
SET(MOO_WEB_CONTACT "http://mooedit.sourceforge.net/contact.html")
SET(MOO_EMAIL "emuntyan@users.sourceforge.net")
SET(MOO_COPYRIGHT "2004-2015 Yevgen Muntyan <${MOO_EMAIL}>")

SET(MOO_PREFS_XML_FILE_NAME "prefs.xml")
SET(MOO_STATE_XML_FILE_NAME "state.xml")
SET(MOO_NAMED_SESSION_XML_FILE_NAME "session-%s.xml")
SET(MOO_SESSION_XML_FILE_NAME "session.xml")
SET(MEDIT_PORTABLE_MAGIC_FILE_NAME "medit-portable")
SET(MEDIT_PORTABLE_DATA_DIR "medit-portable-data")
SET(MEDIT_PORTABLE_CACHE_DIR "medit-portable-cache")

SET(MOO_DATA_DIR "${CMAKE_INSTALL_PREFIX}/share/${MOO_PACKAGE_NAME}")
SET(MOO_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib/${MOO_PACKAGE_NAME}")
SET(MOO_DOC_DIR "${CMAKE_INSTALL_PREFIX}/share/doc/${MOO_PACKAGE_NAME}")
SET(MOO_HELP_DIR "${MOO_DOC_DIR}/help")
SET(MOO_TEXT_LANG_FILES_DIR, "${MOO_DATA_DIR}/language-specs")
SET(MOO_PYTHON_PLUGIN_DIR "${MOO_DATA_DIR}/plugins")
SET(MOO_PYTHON_LIB_DIR "${MOO_DATA_DIR}/python")

if(CMAKE_COMPILER_IS_GNUCXX)
    list(APPEND CMAKE_CXX_FLAGS "-std=c++11")
endif()

if(CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES Debug Release)
    set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
        "Reset the configurations to what we need" FORCE)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED gtk+-2.0)
pkg_check_modules(GMODULE REQUIRED gmodule-2.0)
execute_process(
    COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=glib_genmarshal glib-2.0
    OUTPUT_VARIABLE GLIB_GENMARSHAL
    OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(
    COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=glib_mkenums glib-2.0
    OUTPUT_VARIABLE GLIB_MKENUMS
    OUTPUT_STRIP_TRAILING_WHITESPACE)
find_program(GDK_PIXBUF_CSOURCE gdk-pixbuf-csource)

pkg_check_modules(XLIB x11 xext xrender ice sm)

pkg_check_modules(LIBXML2 REQUIRED libxml-2.0)

find_library(LIBM m)

find_package(Gettext REQUIRED)
SET(GETTEXT_PACKAGE "${MOO_PACKAGE_NAME}")
SET(GETTEXT_PACKAGE_GSV "${MOO_PACKAGE_NAME}-gsv")
# MOO_INTL

find_package(PythonInterp REQUIRED)
SET(MOO_PYTHON "${PYTHON_EXECUTABLE}")

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
add_definitions(-DMOO_COMPILATION=1 -DHAVE_CONFIG_H=1 -DXDG_PREFIX=_moo_edit_xdg -DG_LOG_DOMAIN=\"Moo\")

# zzz SUBDIRS = po po-gsv api doc moo
add_subdirectory(api)
add_subdirectory(moo)
