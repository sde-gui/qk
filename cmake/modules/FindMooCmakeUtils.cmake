MACRO(MOO_EXEC_OR_DIE command)
  EXECUTE_PROCESS(${ARGN}
                  RESULT_VARIABLE __moo_command_result
                  ERROR_VARIABLE __moo_command_error
  )
  IF(NOT __moo_command_result EQUAL 0)
    MESSAGE(FATAL_ERROR "Command ${command} failed: ${__moo_command_error}")
  ENDIF(NOT __moo_command_result EQUAL 0)
ENDMACRO(MOO_EXEC_OR_DIE)

MACRO(MOO_COPY_FILE file1 file2)
  MOO_EXEC_OR_DIE("copy ${file1} ${file2}"
    COMMAND ${CMAKE_COMMAND} -E copy ${file1} ${file2}
  )
ENDMACRO(MOO_COPY_FILE)

MACRO(MOO_COPY_IF_CHANGED file1 file2)
  SET(__moo_do_copy)
  IF(NOT EXISTS ${file2})
    SET(__moo_do_copy 1)
  ELSE(NOT EXISTS ${file2})
    FILE(READ ${file1} __moo_contents1)
    FILE(READ ${file2} __moo_contents2)
    IF(NOT "${__moo_contents1}" STREQUAL "${__moo_contents2}")
      SET(__moo_do_copy 1)
    ENDIF(NOT "${__moo_contents1}" STREQUAL "${__moo_contents2}")
  ENDIF(NOT EXISTS ${file2})
  IF(__moo_do_copy)
    MOO_COPY_FILE(${file1} ${file2})
  ENDIF(__moo_do_copy)
ENDMACRO(MOO_COPY_IF_CHANGED)
