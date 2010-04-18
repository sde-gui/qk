# INPUT_FILE, OUTPUT_FILE

FIND_PROGRAM(GSED gsed)
IF(GSED)
  SET(SED_PROGRAM ${GSED} -r)
ELSEIF(MOO_OS_BSD)
  SET(SED_PROGRAM sed -E)
ELSE(GSED)
  SET(SED_PROGRAM sed -r)
ENDIF(GSED)

MACRO(DO_FILE)
  SET(COMMAND ${SED_PROGRAM} ${ARGN} "${INPUT_FILE}")
  EXECUTE_PROCESS(COMMAND ${COMMAND}
                  RESULT_VARIABLE result
                  ERROR_VARIABLE error
                  OUTPUT_FILE "${OUTPUT_FILE}.tmp")
  IF(NOT result EQUAL 0)
    MESSAGE(FATAL_ERROR "Command ${COMMAND} failed: ${error}")
  ENDIF(NOT result EQUAL 0)
  FILE(RENAME "${OUTPUT_FILE}.tmp" "${OUTPUT_FILE}")
ENDMACRO(DO_FILE)

MACRO(DO_XML)
  DO_FILE(-e "s@<_@<@g" -e "s@</_@</@g")
ENDMACRO(DO_XML)

MACRO(DO_DESKTOP)
  DO_FILE(-e "s@^_(\\w+)=@\\1=@g")
ENDMACRO(DO_DESKTOP)

IF("${INPUT_FILE}" MATCHES ".*[.]xml[.]in$")
  DO_XML()
ELSEIF("${INPUT_FILE}" MATCHES ".*[.](desktop|ini)[.]in$")
  DO_DESKTOP()
ELSE("${INPUT_FILE}" MATCHES ".*[.]xml[.]in$")
  MESSAGE(FATAL_ERROR "Unknown file type: ${INPUT_FILE}")
ENDIF("${INPUT_FILE}" MATCHES ".*[.]xml[.]in$")
