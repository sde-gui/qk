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
SET(MOO_TEXT_LANG_FILES_DIR "${MOO_DATA_DIR}/language-specs")
SET(MOO_PYTHON_PLUGIN_DIR "${MOO_DATA_DIR}/plugins")
SET(MOO_PYTHON_LIB_DIR "${MOO_DATA_DIR}/python")

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

if(WIN32 AND CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES Debug RelWithDebInfo)
    set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
        "Reset the configurations to what we need" FORCE)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

if(WIN32)
    SET(MOO_OS_WIN32 TRUE)
else()
    SET(MOO_OS_UNIX TRUE)
endif()

set(MOO_WIN32_BUILD ${MOO_OS_WIN32} CACHE BOOL "Create a win32 build from hexchat gtk build and friends")

if(NOT MOO_WIN32_BUILD)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK REQUIRED gtk+-2.0)
    pkg_check_modules(GMODULE REQUIRED gmodule-2.0)
    #execute_process(
    #    COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=glib_genmarshal glib-2.0
    #    OUTPUT_VARIABLE GLIB_GENMARSHAL
    #    OUTPUT_STRIP_TRAILING_WHITESPACE
    #    RESULT_VARIABLE RESULT)
    #if(NOT RESULT EQUAL 0)
    #	MESSAGE(FATAL_ERROR "Could not find glib-genmarshal")
    #endif()
    execute_process(
        COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=glib_mkenums glib-2.0
        OUTPUT_VARIABLE GLIB_MKENUMS
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE RESULT)
    if(NOT RESULT EQUAL 0)
	    MESSAGE(FATAL_ERROR "Could not find glib-mkenums")
    endif()

    pkg_check_modules(XLIB x11 xext xrender ice sm)
    pkg_check_modules(LIBXML2 REQUIRED libxml-2.0)
else()
    set(MOO_GTK_DIR "" CACHE PATH "Gtk root dir")
    set(MOO_GTK_DIST_DIR "" CACHE PATH "Gtk binary dir")

    include_directories(SYSTEM "${MOO_GTK_DIR}/include")
    include_directories(SYSTEM "${MOO_GTK_DIR}/include/glib-2.0")
    include_directories(SYSTEM "${MOO_GTK_DIR}/lib/glib-2.0/include")
    include_directories(SYSTEM "${MOO_GTK_DIR}/lib/gtk-2.0/include")
    include_directories(SYSTEM "${MOO_GTK_DIR}/include/gtk-2.0")
    include_directories(SYSTEM "${MOO_GTK_DIR}/include/gdk-pixbuf-2.0")
    include_directories(SYSTEM "${MOO_GTK_DIR}/include/pango-1.0")
    include_directories(SYSTEM "${MOO_GTK_DIR}/include/atk-1.0")
    include_directories(SYSTEM "${MOO_GTK_DIR}/include/cairo")
    include_directories(SYSTEM "${MOO_GTK_DIR}/include/libxml2")

    SET(GTK_LIBRARY_DIRS "${MOO_GTK_DIR}/lib")
    SET(GTK_LIBRARIES libgtk-win32-2.0-0 libatk-1.0-0 libgdk-win32-2.0-0 libgdk_pixbuf-2.0-0
		libpango-1.0-0 libpangocairo-1.0-0 libcairo-2
		libgio-2.0-0 libgobject-2.0-0 libgmodule-2.0-0 libgthread-2.0-0 libglib-2.0-0 libintl-8)
    SET(LIBXML2_LIBRARIES "libxml2-2")

    set(ENV{PATH} "ENV{PATH};${MOO_GTK_DIR}/bin;${MOO_MSYS_BIN_DIR}")

    foreach(var CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
        set(${var} "${${var}} /MP /Zo /wd4996")
        string(REPLACE "/Ob1" "/Ob2" ${var}_RELEASE "${${var}_RELEASE}")
        string(REPLACE "/Ob1" "/Ob2" ${var}_RELWITHDEBINFO "${${var}_RELWITHDEBINFO}")
        string(REPLACE "/MDd" "/MD" ${var}_DEBUG "${${var}_DEBUG}")
        string(REPLACE "/D_DEBUG" "" ${var}_DEBUG "${${var}_DEBUG}")
    endforeach()
endif()

find_program(GLIB_GENMARSHAL glib-genmarshal)
find_program(GDK_PIXBUF_CSOURCE gdk-pixbuf-csource)

#find_library(LIBM m)

find_package(Gettext REQUIRED)
SET(GETTEXT_PACKAGE "${MOO_PACKAGE_NAME}")
SET(GETTEXT_PACKAGE_GSV "${MOO_PACKAGE_NAME}-gsv")
# MOO_INTL

set(PythonInterp_FIND_VERSION TRUE)
set(PythonInterp_FIND_VERSION_MAJOR 2)
find_package(PythonInterp REQUIRED)
SET(MOO_PYTHON "${PYTHON_EXECUTABLE}")

set(MOO_ENABLE_PYTHON TRUE CACHE BOOL "Use python")

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config-cmake.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
add_definitions(-DMOO_COMPILATION=1 -DHAVE_CONFIG_H=1 -DXDG_PREFIX=_moo_edit_xdg -DG_LOG_DOMAIN=\"Moo\")

if(WIN32)
    add_definitions(-DUNICODE=1 -D_UNICODE=1 -D__WIN32__=1 -DSTRICT=1 -D_CRT_SECURE_NO_WARNINGS=1 -D_CRT_NONSTDC_NO_WARNINGS=1)
    set(MOO_OS_WIN32 TRUE)
    add_definitions(-DMOO_NEED_GETTIMEOFDAY=1)
    add_definitions(-DHAVE_UNISTD_H=1 -DENABLE_NLS=1 -DHAVE_BIND_TEXTDOMAIN_CODESET=1)
endif(WIN32)

# zzz SUBDIRS = po po-gsv api doc moo
add_subdirectory(api)
# add_subdirectory(doc)
#add_subdirectory(po)
#add_subdirectory(po-gsv)

if(MOO_WIN32_BUILD)
    include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/moo/mooutils/moowin32/ms")

    set(PYGTK_DEFS_DIR ${MOO_GTK_DIR}/share/pygtk/2.0/defs)
    set(PYGOBJECT_DEFS_DIR ${MOO_GTK_DIR}/share/pygobject/2.0/defs)

    set(PYGTK_INCLUDE_DIR "${MOO_GTK_DIR}/include/pygtk-2.0")
    include_directories(SYSTEM "${PYGTK_INCLUDE_DIR}")
endif()

add_subdirectory(moo)
