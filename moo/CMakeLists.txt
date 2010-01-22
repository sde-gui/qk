INCLUDE(${CMAKE_SOURCE_DIR}/cmake/moo-libs.cmake)

ADD_DEFINITIONS(-DMOO_COMPILATION -DXDG_PREFIX=_moo_xdg_)
INCLUDE_DIRECTORIES(${MOO_SOURCE_DIR}/doc)

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})
ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

MOO_ADD_GENERATED_FILE(moo
  ${CMAKE_CURRENT_BINARY_DIR}/marshals.stamp
  "${CMAKE_CURRENT_BINARY_DIR}/marshals.c;${CMAKE_CURRENT_BINARY_DIR}/marshals.h"
  COMMAND ${MOO_CMAKE_COMMAND} -P ${MOO_SOURCE_DIR}/moo/genmarshal.cmake
  DEPENDS marshals.list genmarshal.cmake
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

SET(MOO_XML2H_PY ${MOO_SOURCE_DIR}/moo/xml2h.py)
SET(MOO_GLADE2C_PY ${MOO_SOURCE_DIR}/moo/glade2c.py)

FIND_PACKAGE(PythonInterp)
IF(NOT PYTHONINTERP_FOUND)
  MESSAGE(FATAL_ERROR "Python not found")
ENDIF(NOT PYTHONINTERP_FOUND)

MACRO(_MOO_FIX_SOURCE_NAME var libname)
  SET(_moo_source_file ${${var}})
  IF(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${_moo_source_file})
    IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${libname}/${_moo_source_file})
      SET(_moo_source_file ${libname}/${_moo_source_file})
    ELSE(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${libname}/${_moo_source_file})
      MESSAGE(FATAL_ERROR "File ${_moo_source_file} does not exist")
    ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${libname}/${_moo_source_file})
  ENDIF(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${_moo_source_file})
  SET(${var} ${_moo_source_file})
ENDMACRO(_MOO_FIX_SOURCE_NAME)

MACRO(MOO_GEN_UIXML libname)
  FOREACH(uifile ${ARGN})
    _MOO_FIX_SOURCE_NAME(uifile ${libname})
    GET_FILENAME_COMPONENT(basename ${uifile} NAME_WE)
    STRING(REPLACE "-" "_" varname ${basename})
    SET(varname ${varname}_ui_xml)
    SET(header ${basename}-ui.h)
    SET(header_full ${CMAKE_CURRENT_BINARY_DIR}/${header})
    SET(header_stamp ${header_full}.stamp)
    MOO_ADD_GENERATED_FILE(${libname}
      ${header_stamp} ${header_full}
      COMMAND ${PYTHON_EXECUTABLE} ${MOO_XML2H_PY}
                                   ${CMAKE_CURRENT_SOURCE_DIR}/${uifile}
                                   ${CMAKE_CURRENT_BINARY_DIR}/${header}
                                   ${varname}
      DEPENDS ${uifile} ${MOO_XML2H_PY}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
  ENDFOREACH(uifile)
ENDMACRO(MOO_GEN_UIXML)

INCLUDE(mooutils/CMakelists.cmake)
INCLUDE(moolua/CMakelists.cmake)
INCLUDE(moofileview/CMakelists.cmake)
INCLUDE(mooedit/CMakelists.cmake)
INCLUDE(mooapp/CMakelists.cmake)

MOO_CHECK_OPTION(MOO_BUILD_SHARED_LIB)

IF(MOO_BUILD_SHARED_LIB)
  MOO_WIN32_RESOURCE(libmoo.rc.in moo)
ENDIF(MOO_BUILD_SHARED_LIB)
IF(WIN32)
  LIST(APPEND moo_all_sources moo-dll.c)
ENDIF(WIN32)

IF(MSVC)
  SET(moo_gendef_args)
  IF(MOO_ENABLE_UNIT_TESTS)
    LIST(APPEND moo_gendef_args test)
  ENDIF(MOO_ENABLE_UNIT_TESTS)
  MOO_ADD_GENERATED_FILE(moo
    ${CMAKE_CURRENT_BINARY_DIR}/moo.def.stamp
    ${CMAKE_CURRENT_BINARY_DIR}/moo.def
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/gendef.py ${CMAKE_CURRENT_BINARY_DIR}/moo.def.tmp ${moo_gendef_args}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/moo.def.tmp ${CMAKE_CURRENT_BINARY_DIR}/moo.def
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/moo.def.tmp
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/moo.def.stamp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/gendef.py
  )
  LIST(APPEND moo_all_sources ${CMAKE_CURRENT_BINARY_DIR}/moo.def)
ENDIF(MSVC)

MOO_ADD_SOURCE_FILE_SET(moo "")

IF(MOO_BUILD_SHARED_LIB)
  ADD_LIBRARY(moo SHARED ${moo_all_sources})
  TARGET_LINK_LIBRARIES(moo ${moo_libs} ${MOO_DEP_LIBS})
  INSTALL(TARGETS moo RUNTIME DESTINATION ${BINDIR} LIBRARY DESTINATION ${LIBDIR})
ELSE(MOO_BUILD_SHARED_LIB)
  ADD_LIBRARY(moo STATIC ${moo_all_sources})
ENDIF(MOO_BUILD_SHARED_LIB)
