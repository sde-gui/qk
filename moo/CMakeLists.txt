# zzz AM_CPPFLAGS = $(MOO_CPPFLAGS) -I$(top_srcdir)/doc/built -Imooutils/glade

FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/genmarshals_h.cmake
"EXECUTE_PROCESS(COMMAND ${GLIB_GENMARSHAL} --prefix=_moo_marshal --header ${CMAKE_CURRENT_SOURCE_DIR}/marshals.list
    RESULT_VARIABLE result
	OUTPUT_VARIABLE output
	ERROR_VARIABLE error
	OUTPUT_FILE marshals.h.tmp)
if(NOT result EQUAL 0)
	MESSAGE(FATAL_ERROR \"Error generating marshals.h: \${output} \${error}\")
endif()
file(RENAME marshals.h.tmp marshals.h)
")
add_custom_command(OUTPUT marshals.h
    COMMAND ${CMAKE_COMMAND} -P genmarshals_h.cmake
    MAIN_DEPENDENCY marshals.list)
LIST(APPEND built_moo_sources marshals.h)

macro(ADD_GXML input)
    string(REPLACE ".glade" "-gxml.h" _gxml_output "${input}")
    string(REPLACE "/glade/" "/" _gxml_output "${_gxml_output}")
    #message(STATUS "_gxml_output: ${_gxml_output}")
    add_custom_command(OUTPUT ${_gxml_output}
        COMMAND ${MOO_PYTHON} ${CMAKE_SOURCE_DIR}/tools/glade2c.py --output=${_gxml_output} ${CMAKE_CURRENT_SOURCE_DIR}/${input}
        MAIN_DEPENDENCY ${input}
        DEPENDS ${CMAKE_SOURCE_DIR}/tools/glade2c.py
        COMMENT "Generating ${_gxml_output} from ${input}")
    list(APPEND built_moo_sources ${_gxml_output})
endmacro(ADD_GXML)

macro(XML2H _xml2h_input _xml2h_output _xml2h_variable)
    add_custom_command(OUTPUT ${_xml2h_output}
        COMMAND ${MOO_PYTHON} ${CMAKE_SOURCE_DIR}/tools/xml2h.py ${CMAKE_CURRENT_SOURCE_DIR}/${_xml2h_input} ${_xml2h_output} ${_xml2h_variable}
        MAIN_DEPENDENCY ${_xml2h_input}
        DEPENDS ${CMAKE_SOURCE_DIR}/tools/xml2h.py
        COMMENT "Generating ${_xml2h_output} from ${_xml2h_input}")
    list(APPEND built_moo_sources ${_xml2h_output})
endmacro(XML2H)

macro(ADD_UI input)
    string(REPLACE ".xml" "-ui.h" _ui_output "${input}")
#     message(STATUS "_ui_output: ${_ui_output}")
    string(REGEX REPLACE ".*/([^/]*)\\.xml" "\\1" _ui_variable "${input}")
    XML2H(${input} ${_ui_output} ${_ui_variable}_ui_xml)
endmacro(ADD_UI)

file(MAKE_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}/mooapp
    ${CMAKE_CURRENT_BINARY_DIR}/mooedit
    ${CMAKE_CURRENT_BINARY_DIR}/moofileview
    ${CMAKE_CURRENT_BINARY_DIR}/mooutils
    ${CMAKE_CURRENT_BINARY_DIR}/moolua
    ${CMAKE_CURRENT_BINARY_DIR}/plugins/usertools
)

include(xdgmime/xdgmime.cmake)
include(mooutils/mooutils.cmake)
include(mooedit/mooedit.cmake)
include(moofileview/moofileview.cmake)
include(gtksourceview/gtksourceview.cmake)
include(eggsmclient/eggsmclient.cmake)
include(mooapp/mooapp.cmake)
include(moolua/moolua.cmake)
if(MOO_ENABLE_PYTHON)
include(moopython/moopython.cmake)
endif()
include(plugins/plugins.cmake)

set(medit_app_sources
    medit-app/mem-debug.h
    medit-app/run-tests.h
    medit-app/parse.h
    medit-app/main.c
)

SET(MEDIT_SOURCES
    ${moo_sources}
    ${built_moo_sources}
)

macro(ADD_MODULE name)
    string(REPLACE "-" "_" _escaped_name "${name}")
    source_group(${name} FILES ${${_escaped_name}_sources} ${built_${_escaped_name}_sources} ${${_escaped_name}_extra_files})
    #source_group(${name} FILES ${${_escaped_name}_sources} ${${_escaped_name}_extra_files})
    #source_group(${name}\\generated FILES ${built_${_escaped_name}_sources})
    LIST(APPEND MEDIT_SOURCES ${${_escaped_name}_sources} ${built_${_escaped_name}_sources})
    LIST(APPEND MEDIT_EXTRA_DIST ${${_escaped_name}_extra_dist} ${${_escaped_name}_extra_files})
endmacro(ADD_MODULE)

ADD_MODULE(xdgmime)
ADD_MODULE(eggsmclient)
ADD_MODULE(gtksourceview)
ADD_MODULE(mooutils)
ADD_MODULE(moofileview)
ADD_MODULE(mooedit)
ADD_MODULE(mooapp)
ADD_MODULE(plugins)
ADD_MODULE(moolua)
if(MOO_ENABLE_PYTHON)
ADD_MODULE(moopython)
endif()
ADD_MODULE(medit-app)

include_directories(${PROJECT_SOURCE_DIR}/moo/mooutils/moowin32/ms)

include_directories(
    ${PROJECT_BINARY_DIR}
    ${GTK_INCLUDE_DIRS}
    ${LIBXML2_INCLUDE_DIRS}
)
link_directories(${GTK_LIBRARY_DIRS} ${GMODULE_LIBRARY_DIRS})

source_group(misc REGULAR_EXPRESSION .*)
source_group("generated\\sources" REGULAR_EXPRESSION ".*\\.c(pp)?$")
source_group("generated\\headers" REGULAR_EXPRESSION ".*\\.h$")
source_group("misc\\glade" REGULAR_EXPRESSION ".*\\.glade$")

add_executable(medit WIN32
    ${MEDIT_SOURCES}
)

target_link_libraries(medit
    ${GTK_LIBRARIES}
    ${GMODULE_LIBRARIES}
    ${LIBXML2_LIBRARIES}
    ${XLIB_LIBRARIES}
    #${LIBM}
    ${PYTHON_LIBRARIES}
    ${moo_libadd}
    ${MEDIT_LIBS}
)

install(TARGETS medit RUNTIME DESTINATION bin)

if(MOO_WIN32_BUILD)
    install(DIRECTORY ${MOO_GTK_DIST_DIR}/bin DESTINATION ${CMAKE_INSTALL_PREFIX})
    install(DIRECTORY ${MOO_GTK_DIST_DIR}/etc DESTINATION ${CMAKE_INSTALL_PREFIX})
    install(DIRECTORY ${MOO_GTK_DIST_DIR}/lib DESTINATION ${CMAKE_INSTALL_PREFIX})
    install(DIRECTORY ${MOO_GTK_DIST_DIR}/share DESTINATION ${CMAKE_INSTALL_PREFIX})
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/plat/win32/gtk-win/extra/medit-data/bin DESTINATION ${CMAKE_INSTALL_PREFIX})
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/plat/win32/gtk-win/extra/medit-data/etc DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()

#zzz
# if MOO_OS_WIN32
# CLEANFILES += medit-app/medit.rc medit-app/medit.res
# medit-app/medit.rc: medit-app/medit.rc.in $(top_builddir)/config.status
# 	$(AM_V_at)$(MKDIR_P) medit-app
# 	$(AM_V_GEN)cd $(top_builddir) && ./config.status --silent --file=moo/medit-app/medit.rc
# medit-app/medit.res: medit-app/medit.rc
# 	$(AM_V_GEN)$(WINDRES) -i medit-app/medit.rc --input-format=rc -o medit-app/medit.res -O coff
# medit_LDADD += medit-app/medit.res
# endif

# zzz
# if MOO_OS_UNIX
# desktopdir = $(datadir)/applications
# desktop_DATA = medit-app/medit.desktop
# medit-app/medit.desktop: medit-app/medit.desktop.in @MOO_INTLTOOL_INI_DEPS@
# 	$(AM_V_at)$(MKDIR_P) medit-app
# 	@MOO_INTLTOOL_INI_CMD@
# CLEANFILES += medit-app/medit.desktop
# endif MOO_OS_UNIX

# zzz
# iconthemedir = $(datadir)/icons/hicolor/48x48/apps
# icontheme_DATA = mooutils/pixmaps/medit.png
# if MOO_ENABLE_INSTALL_HOOKS
# update_icon_cache = gtk-update-icon-cache -f -t $(DESTDIR)$(datadir)/icons/hicolor
# install-data-hook: install-data-hook-gtk-icon-cache
# install-data-hook-gtk-icon-cache:
# 	@if echo "Updating icon cache" && $(update_icon_cache); then	
# 		echo "Done.";						
# 	else								
# 		echo "*** GTK icon cache not updated. After install, run this:";\
# 		echo $(update_icon_cache);				
# 	fi
# uninstall-hook: uninstall-data-hook-gtk-icon-cache
# uninstall-data-hook-gtk-icon-cache:
# 	@if echo "Updating icon cache" && $(update_icon_cache); then echo "Done."; else echo "Failed."; fi
# endif

# zzz include medit-module/Makefile.incl
