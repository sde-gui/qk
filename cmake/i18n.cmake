SET(ENABLE_NLS 1 CACHE BOOL "Enable i18n")

IF(ENABLE_NLS)
  FIND_PACKAGE(Gettext)
  IF(NOT GETTEXT_FOUND)
    MESSAGE("Gettext has not been found, translations will not be enabled")
    SET(ENABLE_NLS)
  ENDIF(NOT GETTEXT_FOUND)
ENDIF(ENABLE_NLS)

IF(ENABLE_NLS)
  MOO_DEFINE_H(ENABLE_NLS "Enable i18n")
ENDIF(ENABLE_NLS)

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
#       ADD_CUSTOM_COMMAND(TARGET translations-${catalogname}
#                          COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} --check -o ${_moo_gmo_file} ${_moo_po_file}
#                          DEPENDS ${_moo_po_file})
      INSTALL(FILES ${_moo_gmo_file} DESTINATION ${LOCALEDIR}/${_moo_lang_code}/LC_MESSAGES/ RENAME ${catalogname}.mo)
    ENDFOREACH(_moo_po_file)
    SET_SOURCE_FILES_PROPERTIES(${_moo_gmo_files} PROPERTIES GENERATED 1 EXTERNAL_OBJECT 1)
    ADD_CUSTOM_TARGET(translations-${catalogname} ALL SOURCES ${_moo_gmo_files})
  ENDIF(ENABLE_NLS)
ENDMACRO(MOO_ADD_MSG_CATALOG)
