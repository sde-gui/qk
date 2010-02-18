SET(SMCLIENT_SOURCES
  eggsmclient.c
  eggsmclient.h
  eggsmclient-mangle.h
  eggsmclient-private.h
)

IF(WIN32)
  LIST(APPEND SMCLIENT_SOURCES eggsmclient-win32.c)
ELSEIF(APPLE)
  LIST(APPEND SMCLIENT_SOURCES eggsmclient-dummy.c)
ELSE(WIN32)
  MOO_ADD_COMPILE_DEFINITIONS(-DEGG_SM_CLIENT_BACKEND_XSMP)
  LIST(APPEND SMCLIENT_SOURCES eggsmclient-xsmp.c eggdesktopfile.h eggdesktopfile.c)
ENDIF(WIN32)

MOO_ADD_MOO_CODE_MODULE(smclient)

# -%- strip:true -%-
