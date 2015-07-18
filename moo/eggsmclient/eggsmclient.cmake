SET(eggsmclient_sources
    eggsmclient/eggsmclient.c
    eggsmclient/eggsmclient.h
    eggsmclient/eggsmclient-mangle.h
    eggsmclient/eggsmclient-private.h
)

SET(eggsmclient_extra_dist
    eggsmclient/eggsmclient-win32.c
    eggsmclient/eggsmclient-dummy.c
    eggsmclient/eggsmclient-xsmp.c
    eggsmclient/eggdesktopfile.h
    eggsmclient/eggdesktopfile.c
)

if(MOO_OS_WIN32)
LIST(APPEND eggsmclient_sources eggsmclient/eggsmclient-win32.c)
elseif(MOO_OS_DARWIN)
LIST(APPEND eggsmclient_sources eggsmclient/eggsmclient-dummy.c)
else(MOO_OS_WIN32)
add_definitions(-DEGG_SM_CLIENT_BACKEND_XSMP)
LIST(APPEND eggsmclient_sources eggsmclient/eggsmclient-xsmp.c eggsmclient/eggdesktopfile.h eggsmclient/eggdesktopfile.c)
endif(MOO_OS_WIN32)

# -%- strip:true -%-
