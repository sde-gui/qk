MOO_OPTION(ENABLE_NLS TRUE "Enable i18n" HEADER TRUE)

IF(ENABLE_NLS)
  FIND_PACKAGE(Gettext)
  IF(NOT GETTEXT_FOUND)
    MOO_ERROR("Gettext has not been found, translations will not be enabled")
  ENDIF(NOT GETTEXT_FOUND)
ENDIF(ENABLE_NLS)

MACRO(MOO_TRANSFORM_INI_FILES outvar)
  FOREACH(_moo_ini_in_in_file ${ARGN})
    SET(_moo_ini_in_in_file_abs ${CMAKE_CURRENT_SOURCE_DIR}/${_moo_ini_in_in_file})
    STRING(REPLACE ".ini.in.in" ".ini.in" _moo_ini_in_file ${_moo_ini_in_in_file})
    SET(_moo_ini_in_file ${CMAKE_CURRENT_BINARY_DIR}/${_moo_ini_in_file})
    CONFIGURE_FILE(${_moo_ini_in_in_file} ${_moo_ini_in_file})
    STRING(REPLACE ".ini.in.in" ".ini" _moo_ini_file ${_moo_ini_in_in_file})
    SET(_moo_ini_file ${_moo_ini_file})
    LIST(APPEND ${outvar} ${CMAKE_CURRENT_BINARY_DIR}/${_moo_ini_file})
    IF(ENABLE_NLS)
      ADD_CUSTOM_COMMAND(OUTPUT ${_moo_ini_file}
                         COMMAND ${INTLTOOL_MERGE_COMMAND} -d -u -c ${CMAKE_BINARY_DIR}/po/.intltool-merge-cache ${CMAKE_SOURCE_DIR}/po ${_moo_ini_in_file} ${_moo_ini_file}
                         DEPENDS ${_moo_ini_in_file})
    ELSE(ENABLE_NLS)
      ADD_CUSTOM_COMMAND(OUTPUT ${_moo_ini_file}
                         COMMAND ${MOO_CMAKE_COMMAND} -D MOO_OS_BSD=${MOO_OS_BSD}
                                                      -D INPUT_FILE=${_moo_ini_in_file}
                                                      -D OUTPUT_FILE=${_moo_ini_file}
                                                      -P ${CMAKE_SOURCE_DIR}/moo/mooutils/moo-intltool-merge.cmake
                         DEPENDS ${_moo_ini_in_file})
    ENDIF(ENABLE_NLS)
  ENDFOREACH(_moo_ini_in_in_file)
ENDMACRO(MOO_TRANSFORM_INI_FILES)

MACRO(MOO_ADD_MSG_CATALOG catalogname dir)
  IF(ENABLE_NLS)
    FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${dir})
    FILE(GLOB _moo_po_files ${dir}/*.po)
    SET(_moo_langs)
    SET(_moo_gmo_files)
    FOREACH(_moo_po_file ${_moo_po_files})
      GET_FILENAME_COMPONENT(_moo_lang_code ${_moo_po_file} NAME_WE)
      LIST(APPEND _moo_langs ${_moo_lang_code})
      SET(_moo_gmo_file ${CMAKE_BINARY_DIR}/${dir}/${_moo_lang_code}.gmo)
      LIST(APPEND _moo_gmo_files ${_moo_gmo_file})
      ADD_CUSTOM_COMMAND(OUTPUT ${_moo_gmo_file}
                         COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} --check -o ${_moo_gmo_file} ${_moo_po_file}
                         DEPENDS ${_moo_po_file})
      INSTALL(FILES ${_moo_gmo_file} DESTINATION ${LOCALEDIR}/${_moo_lang_code}/LC_MESSAGES/ RENAME ${catalogname}.mo)
    ENDFOREACH(_moo_po_file)
    SET_SOURCE_FILES_PROPERTIES(${_moo_gmo_files} PROPERTIES GENERATED 1 EXTERNAL_OBJECT 1)
    ADD_CUSTOM_TARGET(intl-${catalogname} ALL SOURCES ${_moo_gmo_files})
  ENDIF(ENABLE_NLS)
ENDMACRO(MOO_ADD_MSG_CATALOG)

IF(ENABLE_NLS)
  IF(WIN32)
    MACRO(_MOO_FIND_INTLTOOL_VAR var script)
      SET(${var} $ENV{${var}})
      IF(NOT "${var}")
        FIND_FILE(${var} ${script})
      ENDIF(NOT "${var}")
    ENDMACRO(_MOO_FIND_INTLTOOL_VAR)
    FOREACH(_moo_i18n_tool update extract merge)
      STRING(TOUPPER ${_moo_i18n_tool} _moo_i18n_TOOL)
      _MOO_FIND_INTLTOOL_VAR(INTLTOOL_${_moo_i18n_TOOL} intltool-${_moo_i18n_tool})
      SET(__MOO_INTLTOOL_SCRIPT ${INTLTOOL_${_moo_i18n_TOOL}})
      CONFIGURE_FILE(${MOO_SOURCE_DIR}/plat/win32/intltool-wrapper.bat.in ${MOO_BINARY_DIR}/intltool-${_moo_i18n_tool}-wrapper.bat)
      SET(INTLTOOL_${_moo_i18n_TOOL}_COMMAND ${MOO_BINARY_DIR}/intltool-${_moo_i18n_tool}-wrapper.bat)
    ENDFOREACH(_moo_i18n_tool)
  ELSE(WIN32)
    FIND_PROGRAM(INTLTOOL_UPDATE_COMMAND intltool-update)
    FIND_PROGRAM(INTLTOOL_MERGE_COMMAND intltool-merge)
    FIND_PROGRAM(INTLTOOL_EXTRACT_COMMAND intltool-extract)
  ENDIF(WIN32)

  SET(MOO_PO_DIR ${MOO_SOURCE_DIR}/po)

  ADD_CUSTOM_TARGET(
    intl-check
    ${INTLTOOL_UPDATE_COMMAND} --gettext-package=moo -m
    WORKING_DIRECTORY ${MOO_PO_DIR}
  )

  ADD_CUSTOM_TARGET(
    intl-pot
    COMMAND ${INTLTOOL_UPDATE_COMMAND} --gettext-package=moo -p
    WORKING_DIRECTORY ${MOO_PO_DIR}
  )

  FILE(STRINGS ${MOO_PO_DIR}/LINGUAS MOO_PO_LINGUAS)
  ADD_CUSTOM_TARGET(intl-update-po)
  FOREACH(_moo_lang ${MOO_PO_LINGUAS})
    ADD_CUSTOM_TARGET(
      intl-update-${_moo_lang}-po
      ${INTLTOOL_UPDATE_COMMAND} --gettext-package=moo -d ${_moo_lang}
      WORKING_DIRECTORY ${MOO_PO_DIR}
    )
    ADD_DEPENDENCIES(intl-update-po intl-update-${_moo_lang}-po)
  ENDFOREACH(_moo_lang)
ENDIF(ENABLE_NLS)
