MOO_OPTION(MOO_ENABLE_PYTHON TRUE "Enable Python" HEADER TRUE)

SET(PYTHON_DEV_FOUND FALSE)

MACRO(MOO_GET_PYTHON_SYSCONFIG_VAR var value error)
  SET(${error} FALSE)
  EXECUTE_PROCESS(COMMAND "${PYTHON_EXECUTABLE}" -c "import distutils.sysconfig; print distutils.sysconfig.get_config_vars('${var}')[0]"
                  RESULT_VARIABLE _moo_gpsv_result
                  OUTPUT_VARIABLE _moo_gpsv_output
                  ERROR_VARIABLE _moo_gpsv_error
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  IF(_moo_gpsv_result EQUAL 0)
    SET(${value} "${_moo_gpsv_output}")
  ELSE(_moo_gpsv_result EQUAL 0)
    SET(${error} TRUE)
  ENDIF(_moo_gpsv_result EQUAL 0)
ENDMACRO(MOO_GET_PYTHON_SYSCONFIG_VAR)

IF(MOO_ENABLE_PYTHON)
  PKG_CHECK_MODULES(PYGTK pygtk-2.0)
  IF(PYGTK_FOUND)
    _MOO_GET_PKG_CONFIG_VARIABLE(PYGTK_DEFS_DIR defsdir pygtk-2.0)
    _MOO_GET_PKG_CONFIG_VARIABLE(PYGTK_CODEGEN_DIR codegendir pygtk-2.0)
    MOO_GET_PYTHON_SYSCONFIG_VAR("INCLUDEPY" _moo_fp_includepy _moo_fp_error)
    IF(NOT _moo_fp_error)
      MOO_GET_PYTHON_SYSCONFIG_VAR("VERSION" _moo_fp_pyversion _moo_fp_error)
    ENDIF(NOT _moo_fp_error)
    IF(NOT _moo_fp_error AND WIN32)
      MOO_GET_PYTHON_SYSCONFIG_VAR("prefix" _moo_fp_pyprefix _moo_fp_error)
    ENDIF(NOT _moo_fp_error AND WIN32)
    IF(NOT _moo_fp_error)
      SET(PYTHON_LIBRARIES "python${_moo_fp_pyversion}")
      FILE(TO_CMAKE_PATH "${_moo_fp_includepy}" PYTHON_INCLUDE_DIRS)
      FILE(TO_CMAKE_PATH "${_moo_fp_pyprefix}/libs" PYTHON_LIBRARY_DIRS)
      SET(PYTHON_DEV_FOUND TRUE)
    ENDIF(NOT _moo_fp_error)
  ENDIF(PYGTK_FOUND)
ENDIF(MOO_ENABLE_PYTHON)

MOO_DEBUG(STATUS "PYTHON_DEV_FOUND = ${PYTHON_DEV_FOUND}")

IF(PYTHON_DEV_FOUND)
  MOO_DEBUG(STATUS "PYTHON_INCLUDE_DIRS = ${PYTHON_INCLUDE_DIRS}")
  MOO_DEBUG(STATUS "PYTHON_LIBRARIES = ${PYTHON_LIBRARIES}")
  MOO_DEBUG(STATUS "PYTHON_LIBRARY_DIRS = ${PYTHON_LIBRARY_DIRS}")
  LIST(APPEND MOO_DEP_LIBS ${PYTHON_LIBRARIES})
  LINK_DIRECTORIES(${PYTHON_LIBRARY_DIRS} ${PYGTK_LIBRARY_DIRS})
ELSE(PYTHON_DEV_FOUND)
  SET(MOO_ENABLE_PYTHON FALSE)
ENDIF(PYTHON_DEV_FOUND)

# -%- strip:true, indent-width:2 -%-
