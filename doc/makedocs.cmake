MACRO(MOO_EXEC_OR_DIE command)
  EXECUTE_PROCESS(${ARGN}
                  RESULT_VARIABLE __moo_command_result
                  ERROR_VARIABLE __moo_command_error
  )
  IF(NOT __moo_command_result EQUAL 0)
    MESSAGE(FATAL_ERROR "Command ${command} failed: ${__moo_command_error}")
  ENDIF(NOT __moo_command_result EQUAL 0)
ENDMACRO(MOO_EXEC_OR_DIE)

MACRO(RENAME_OR_DIE src dst)
  MOO_EXEC_OR_DIE("copy ${src} ${dst}" COMMAND ${CMAKE_COMMAND} -E copy ${src} ${dst})
  MOO_EXEC_OR_DIE("remove ${src}" COMMAND ${CMAKE_COMMAND} -E remove ${src})
ENDMACRO(RENAME_OR_DIE)

MACRO(RENAME_IF_DIFFERENT_OR_DIE src dst)
  MOO_EXEC_OR_DIE("copy ${src} ${dst}" COMMAND ${CMAKE_COMMAND} -E copy_if_different ${src} ${dst})
  MOO_EXEC_OR_DIE("remove ${src}" COMMAND ${CMAKE_COMMAND} -E remove ${src})
ENDMACRO(RENAME_IF_DIFFERENT_OR_DIE)

GET_FILENAME_COMPONENT(DOC_DIR . ABSOLUTE)

MOO_EXEC_OR_DIE(txt2tags COMMAND ${PYTHON_EXECUTABLE} ${TXT2TAGS} --target=man --outfile=${MAN_OUTPUT_FILE}.tmp ${MAN_INPUT_FILE})
RENAME_OR_DIE(${MAN_OUTPUT_FILE}.tmp ${MAN_OUTPUT_FILE})

MOO_EXEC_OR_DIE(txt2tags COMMAND ${PYTHON_EXECUTABLE} ${TXT2TAGS} --outfile=${OUTPUT_FILE}.tmp ${INPUT_FILE})

FILE(MAKE_DIRECTORY ${OUTPUT_DIR})
MOO_EXEC_OR_DIE(splity COMMAND ${PERL_EXECUTABLE} -I${DOC_DIR} ${DOC_DIR}/splity/splity.pl
                              -index ${DOC_DIR}/splity/index-template.html
                              -page ${DOC_DIR}/splity/page-template.html ${OUTPUT_FILE}.tmp
                WORKING_DIRECTORY ${OUTPUT_DIR})

SET(sections)
FILE(STRINGS ${OUTPUT_FILE}.tmp lines REGEX "##(.+)##")
FOREACH(line ${lines})
  SET(section)
  STRING(REGEX REPLACE "^.*##(.+)##.*$" "\\1" section "${line}")
  IF(section)
    LIST(APPEND sections ${section})
  ENDIF(section)
ENDFOREACH(line)

FILE(WRITE  ${SECTIONS_FILE}.tmp "#ifndef HELP_SECTIONS_H\n")
FILE(APPEND ${SECTIONS_FILE}.tmp "#define HELP_SECTIONS_H\n\n")
FOREACH(section ${sections})
  IF(section MATCHES "fake-")
    STRING(REPLACE "fake-" "" section ${section})
    STRING(TOUPPER ${section} SECTION)
    STRING(REPLACE "-" "_" SECTION ${SECTION})
    SET(section)
  ELSE(section MATCHES "fake-")
    STRING(TOUPPER ${section} SECTION)
    STRING(REPLACE "-" "_" SECTION ${SECTION})
  ENDIF(section MATCHES "fake-")
  FILE(APPEND ${SECTIONS_FILE}.tmp "#define HELP_SECTION_${SECTION} \"${section}\"\n")
ENDFOREACH(section)
FILE(APPEND ${SECTIONS_FILE}.tmp "\n#endif /* HELP_SECTIONS_H */\n")

RENAME_IF_DIFFERENT_OR_DIE(${SECTIONS_FILE}.tmp ${SECTIONS_FILE})
RENAME_OR_DIE(${OUTPUT_FILE}.tmp ${OUTPUT_FILE})
