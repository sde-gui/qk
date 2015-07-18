SET(mooapp_sources 
    mooapp/mooapp.cmake
    mooapp/mooappabout.c
    mooapp/mooappabout.h
    mooapp/mooapp.c
    mooapp/mooapp.h
    mooapp/mooapp-accels.h
    mooapp/mooapp-info.h
    mooapp/mooapp-private.h
    mooapp/moohtml.h
    mooapp/moohtml.c
    mooapp/moolinklabel.h
    mooapp/moolinklabel.c
)

foreach(input_file
    mooapp/glade/mooappabout-dialog.glade
    mooapp/glade/mooappabout-license.glade
    mooapp/glade/mooappabout-credits.glade
)
    ADD_GXML(${input_file})
endforeach(input_file)

add_custom_command(OUTPUT mooapp-credits.h
    COMMAND ${MOO_PYTHON} ${CMAKE_SOURCE_DIR}/tools/xml2h.py ${CMAKE_SOURCE_DIR}/THANKS mooapp-credits.h MOO_APP_CREDITS
    MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/THANKS
    DEPENDS ${CMAKE_SOURCE_DIR}/tools/xml2h.py)
list(APPEND built_mooapp_sources mooapp-credits.h)
