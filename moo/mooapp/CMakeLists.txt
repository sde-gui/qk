SET(MOOAPP_SOURCES
  mooappabout.c
  mooappabout.h
  mooapp.c
  mooapp.h
  mooapp-accels.h
  mooapp-info.h
  mooapp-private.h
  moohtml.h
  moohtml.c
  moolinklabel.h
  moolinklabel.c
)

MOO_GEN_GXML(
  glade/mooappabout-dialog.glade
  glade/mooappabout-license.glade
  glade/mooappabout-credits.glade
)

MOO_ADD_GENERATED_FILE(
  ${CMAKE_CURRENT_BINARY_DIR}/mooapp-credits.h.stamp ${CMAKE_CURRENT_BINARY_DIR}/mooapp-credits.h
  COMMAND ${PYTHON_EXECUTABLE} ${MOO_XML2H_PY} ${MOO_SOURCE_DIR}/THANKS ${CMAKE_CURRENT_BINARY_DIR}/mooapp-credits.h MOO_APP_CREDITS
  DEPENDS ${MOO_SOURCE_DIR}/THANKS ${MOO_XML2H_PY}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Generating mooapp-credits.h"
)

MOO_ADD_MOO_CODE_MODULE(mooapp)

ADD_SUBDIRECTORY(smclient)
