set(PythonLibs_FIND_VERSION TRUE)
set(PythonLibs_FIND_VERSION_MAJOR 2)
find_package(PythonLibs REQUIRED)

add_definitions(-DMOO_ENABLE_PYTHON=1)
include_directories(${PYTHON_INCLUDE_DIRS})

SET(moopython_sources
    moopython/pygtk/moo-pygtk.c
    moopython/pygtk/moo-pygtk.h
)

SET(built_moopython_sources
    moopython/pygtk/moo-mod.h
)

set(gendefs_files
    ${CMAKE_SOURCE_DIR}/api/gendefs.py
    ${CMAKE_SOURCE_DIR}/api/mpi/__init__.py
    ${CMAKE_SOURCE_DIR}/api/mpi/module.py
    ${CMAKE_SOURCE_DIR}/api/mpi/defswriter.py
)

set(moo_override_files
    moopython/pygtk/mooutils.override
    moopython/pygtk/moopaned.override
    moopython/pygtk/mooedit.override
    moopython/pygtk/moo.override
)

set(moopython_extra_dist
    ${moo_override_files}
    moopython/pygtk/codebefore.c
    moopython/pygtk/codeafter.c
    moopython/pygtk/moo.py
)

list(APPEND built_moopython_sources
    moopython/pygtk/moo-mod.c
    moopython/pygtk/moo-mod.h
)

if(WIN32)
set(codegen_platform --platform win32)
endif()

list(APPEND built_moopython_sources moopython/pygtk/moo.defs)
add_custom_command(OUTPUT moopython/pygtk/moo.defs
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/getoutput.py moopython/pygtk/moo.defs
        ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/api/gendefs.py ${CMAKE_SOURCE_DIR}/api/moo.xml
    DEPENDS ${gendefs_files} ${CMAKE_SOURCE_DIR}/api/moo.xml)

set(codegen_files
    moopython/codegen/codegen.py
    moopython/codegen/argtypes.py
    moopython/codegen/argtypes_m.py
    moopython/codegen/reversewrapper.py
)

set(codegen_script ${CMAKE_CURRENT_SOURCE_DIR}/moopython/codegen/codegen.py)
set(codegen ${PYTHON_EXECUTABLE} ${codegen_script} ${codegen_platform}
    --codebefore ${CMAKE_CURRENT_SOURCE_DIR}/moopython/pygtk/codebefore.c
    --codeafter ${CMAKE_CURRENT_SOURCE_DIR}/moopython/pygtk/codeafter.c
)

add_custom_command(OUTPUT moopython/pygtk/moo-mod.c
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/getoutput.py moopython/pygtk/moo-mod.c
        ${codegen} --prefix _moo
            --load-types ${CMAKE_CURRENT_SOURCE_DIR}/moopython/codegen/argtypes_m.py
            --register ${PYGOBJECT_DEFS_DIR}/gio-types.defs
            --register ${PYGTK_DEFS_DIR}/gtk-types.defs
            --register ${PYGTK_DEFS_DIR}/gdk-types.defs
            --override ${CMAKE_CURRENT_SOURCE_DIR}/moopython/pygtk/moo.override
            --outfilename moopython/pygtk/moo-mod.c
            moopython/pygtk/moo.defs
    MAIN_DEPENDENCY moopython/pygtk/moo.defs
    DEPENDS ${moo_override_files} ${codegen_files}
        moopython/pygtk/codebefore.c
        moopython/pygtk/codeafter.c
)

XML2H(moopython/pygtk/moo.py moopython/pygtk/moo-mod.h MOO_PY)

set(moo_python_plugins
    terminal
    python
)

# set(moo_python_ini_in_in_files
#     moopython/plugins/terminal.ini.in.in
#     moopython/plugins/python.ini.in.in
# )

# set(moo_python_plugins
#     moopython/plugins/terminal.py
#     moopython/plugins/python.py
# )

set(moo_python_lib_files
    moopython/plugins/lib/pyconsole.py
    moopython/plugins/lib/insert_date_and_time.py
)

set(moo_python_lib_medit_files
    moopython/plugins/medit/__init__.py
    moopython/plugins/medit/runpython.py
)

list(APPEND moopython_extra_dist
    ${moo_python_ini_in_in_files}
    ${moo_python_plugins}
    ${moo_python_lib_files}
    ${moo_python_lib_medit_files}
)

if(MOO_ENABLE_PYTHON)
    foreach(plugin ${moo_python_plugins})
        configure_file(moopython/plugins/${plugin}.ini.in moopython/plugins/${plugin}.ini)
#         add_custom_command(OUTPUT moopython/plugins/${plugin}.ini
#             COMMAND ${INTLTOOL} ${CMAKE_CURRENT_BINARY_DIR}/moopython/plugins/${plugin}.ini.in
#                 ${CMAKE_CURRENT_BINARY_DIR}/moopython/plugins/${plugin}.ini
#             MAIN_DEPENDENCY ${CMAKE_CURRENT_BINARY_DIR}/moopython/plugins/${plugin}.ini.in)
        list(APPEND built_moopython_sources
            ${CMAKE_CURRENT_BINARY_DIR}/moopython/plugins/${plugin}.ini
        )
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/moopython/plugins/${plugin}.ini moopython/plugins/${plugin}.py
            DESTINATION ${MOO_PYTHON_PLUGIN_DIR})
    endforeach()

    install(FILES ${moo_python_lib_files} DESTINATION ${MOO_PYTHON_LIB_DIR})
    install(FILES ${moo_python_lib_medit_files} DESTINATION ${MOO_PYTHON_LIB_DIR}/medit)
endif()

list(APPEND moopython_sources
    moopython/moopython-pygtkmod.h
    moopython/moopython-builtin.h
    moopython/moopython-builtin.c
    moopython/moopython-api.h
    moopython/moopython-loader.h
    moopython/moopython-loader.c
    moopython/moopython-utils.h
    moopython/moopython-utils.c
    moopython/moopython-tests.h
    moopython/moopython-tests.c
)

list(APPEND moopython_sources
    moopython/medit-python.h
    moopython/medit-python.c
)

XML2H(moopython/medit-python-init.py moopython/medit-python-init.h MEDIT_PYTHON_INIT_PY)

# EXTRA_DIST +=					\
# 	$(moopython_sources)			\
# 	moopython/codegen/__init__.py		\
# 	moopython/codegen/argtypes.py		\
# 	moopython/codegen/argtypes_m.py		\
# 	moopython/codegen/codegen.py		\
# 	moopython/codegen/definitions.py	\
# 	moopython/codegen/defsparser.py		\
# 	moopython/codegen/docgen.py		\
# 	moopython/codegen/mergedefs.py		\
# 	moopython/codegen/mkskel.py		\
# 	moopython/codegen/override.py		\
# 	moopython/codegen/reversewrapper.py	\
# 	moopython/codegen/scmexpr.py
