FOREACH(file context.cfg menu.cfg)
  IF(WIN32)
    INSTALL(FILES win32-${file} DESTINATION ${MOO_DATA_DIR} RENAME ${file})
  ELSE(WIN32)
    INSTALL(FILES unix-${file} DESTINATION ${MOO_DATA_DIR} RENAME ${file})
  ENDIF(WIN32)
ENDFOREACH(file)
FOREACH(file filters.xml)
  INSTALL(FILES ${file} DESTINATION ${MOO_DATA_DIR})
ENDFOREACH(file)

LIST(APPEND MOOUSERTOOLS_SOURCES
  moousertools.c
  moousertools.h
  moousertools-prefs.c
  moousertools-prefs.h
  moocommand.c
  moocommand.h
  moocommand-private.h
  moocommand-lua.c
  moocommand-lua.h
  moocommanddisplay.c
  moocommanddisplay.h
  moooutputfilterregex.c
  moooutputfilterregex.h
  mookeyfile.c
  mookeyfile.h
)

IF(NOT WIN32)
  LIST(APPEND MOOUSERTOOLS_SOURCES
    moocommand-sh.c
    moocommand-sh.h
  )
ENDIF(NOT WIN32)

MOO_GEN_GXML(
  glade/mooedittools-exe.glade
  glade/mooedittools-lua.glade
  glade/moousertools.glade
)

MOO_GEN_ENUMS(moousertools)

MOO_ADD_MOO_CODE_MODULE(moousertools)

# -%- strip:true -%-
