if(WIN32)
set(genplatxml_args --enable=win32)
else()
set(genplatxml_args --enable=unix)
endif()
if(MOO_ENABLE_PYTHON)
list(APPEND genplatxml_args --enable=python)
endif()

set(TOOLS_XML_FILES)

foreach(f menu context)
    list(APPEND TOOLS_XML_FILES ${CMAKE_CURRENT_BINARY_DIR}/plugins/usertools/${f}.xml)
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/plugins/usertools/${f}.xml
        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/getoutput.py plugins/usertools/${f}.xml
            ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/plugins/usertools/genplatxml.py ${genplatxml_args}
                ${CMAKE_CURRENT_SOURCE_DIR}/plugins/usertools/${f}-tmpl.xml
        MAIN_DEPENDENCY plugins/usertools/${f}-tmpl.xml
        DEPENDS plugins/usertools/genplatxml.py)
endforeach()

install(FILES
    plugins/usertools/filters.xml
    ${TOOLS_XML_FILES}
DESTINATION ${MOO_DATA_DIR})

LIST(APPEND plugins_sources
    plugins/usertools/moousertools.c
    plugins/usertools/moousertools.h
    plugins/usertools/moousertools-prefs.c
    plugins/usertools/moousertools-prefs.h
    plugins/usertools/moocommand.c
    plugins/usertools/moocommand.h
    plugins/usertools/moocommand-private.h
    plugins/usertools/moocommanddisplay.c
    plugins/usertools/moocommanddisplay.h
    plugins/usertools/moooutputfilterregex.c
    plugins/usertools/moooutputfilterregex.h
    plugins/usertools/moousertools-enums.h
    plugins/usertools/moousertools-enums.c
    plugins/usertools/moocommand-exe.c
    plugins/usertools/moocommand-exe.h
    plugins/usertools/moocommand-script.cpp
    plugins/usertools/moocommand-script.h
    ${TOOLS_XML_FILES}
)

foreach(input_file
	plugins/usertools/glade/mooedittools-exe.glade
	plugins/usertools/glade/mooedittools-script.glade
	plugins/usertools/glade/moousertools.glade
)
    ADD_GXML(${input_file})
endforeach(input_file)

LIST(APPEND plugins_extra_files plugins/usertools/lua-tool-setup.lua)

XML2H(plugins/usertools/lua-tool-setup.lua plugins/usertools/lua-tool-setup.h LUA_TOOL_SETUP_LUA)

if(MOO_ENABLE_PYTHON)
XML2H(plugins/usertools/python-tool-setup.py plugins/usertools/python-tool-setup.h PYTHON_TOOL_SETUP_PY)
endif()
