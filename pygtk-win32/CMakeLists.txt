link_directories(${GTK_LIBRARY_DIRS})

set(PythonLibs_FIND_VERSION TRUE)
set(PythonLibs_FIND_VERSION_MAJOR 2)
find_package(PythonLibs REQUIRED)

include_directories(${PYTHON_INCLUDE_DIRS})

set(pygobject_major_version 2)
set(pygobject_minor_version 28)
set(pygobject_micro_version 6)
set(pygobject_version ${pygobject_major_version}.${pygobject_minor_version}.${pygobject_micro_version})
set(PYGOBJECT_SRC_DIR pygobject-${pygobject_version})

set(PYCAIRO_SRC_DIR py2cairo-1.10.0)

set(MEDIT_PY_LIBDIR bin/lib/site-packages)

foreach(var CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
    set(${var} "${${var}} /wd4018 /wd4090 /wd4101 /wd4244 /wd4996")
endforeach()

macro(DEFS2C)
    set(_srcdir ${ARGV0})
    set(_prefix ${ARGV1})
    set(_output ${ARGV2})
    set(_defs_file ${ARGV3})

    set(_dep_defs_files)
    set(_defs_files ${_defs_file} ${DEFS2C_ADDITIONAL_DEFS_FILES})
    set(DEFS2C_ADDITIONAL_DEFS_FILES)
    foreach(_d ${_defs_files})
        list(APPEND _dep_defs_files ${_srcdir}/${_d})
    endforeach()
    #MESSAGE(STATUS "_defs_file: ${_defs_file}")
    #MESSAGE(STATUS "_dep_defs_files: ${_dep_defs_files}")
    #MESSAGE(STATUS "${CMAKE_CURRENT_SOURCE_DIR}/${_srcdir}")

    set(_additional_args "${ARGV}")
    list(REMOVE_AT _additional_args 0 1 2 3)
    #MESSAGE(STATUS "_additional_args: ${_additional_args}")

    #MESSAGE(STATUS "ARGV: ${ARGV}")
    #MESSAGE(STATUS "CMD LINE: ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/getoutput.py ${_output} ${CMAKE_CURRENT_SOURCE_DIR}/${PYGOBJECT_SRC_DIR}/codegen/codegen.py")
    #MESSAGE(STATUS "SRCDIR: ${CMAKE_CURRENT_SOURCE_DIR}/${_srcdir}")
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_output}
        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/getoutput.py ${CMAKE_CURRENT_BINARY_DIR}/${_output}
            ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/pygtk-win32/pygobject-${pygobject_version}/codegen/codegen.py
            --py_ssize_t-clean
            ${_additional_args}
            --prefix py${_prefix} ${_defs_file}
            --outfilename ${_output}
        MAIN_DEPENDENCY ${_srcdir}/${_defs_file}
        DEPENDS ${CMAKE_SOURCE_DIR}/getoutput.py ${_dep_defs_files}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${_srcdir})
endmacro(DEFS2C)

add_subdirectory(pycairo)
add_subdirectory(pygobject)
add_subdirectory(pygtk)
