LIST(APPEND moo_sources
    eggsmclient/eggsmclient.c
    eggsmclient/eggsmclient.h
    eggsmclient/eggsmclient-mangle.h
    eggsmclient/eggsmclient-private.h
)

if(MOO_OS_WIN32)
LIST(APPEND moo_sources eggsmclient/eggsmclient-win32.c)
elseif(MOO_OS_DARWIN)
LIST(APPEND moo_sources eggsmclient/eggsmclient-dummy.c)
else(MOO_OS_WIN32)
add_definitions(-DEGG_SM_CLIENT_BACKEND_XSMP)
LIST(APPEND moo_sources eggsmclient/eggsmclient-xsmp.c eggsmclient/eggdesktopfile.h eggsmclient/eggdesktopfile.c)
endif(MOO_OS_WIN32)

# -%- strip:true -%-
