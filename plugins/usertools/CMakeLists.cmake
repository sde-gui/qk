FOREACH(file context.cfg menu.cfg)
  IF(WIN32)
    INSTALL(FILES usertools/win32-${file} DESTINATION ${MOO_DATA_DIR} RENAME ${file})
  ELSE(WIN32)
    INSTALL(FILES usertools/unix-${file} DESTINATION ${MOO_DATA_DIR} RENAME ${file})
  ENDIF(WIN32)
ENDFOREACH(file)
FOREACH(file filters.xml)
  INSTALL(FILES usertools/${file} DESTINATION ${MOO_DATA_DIR})
ENDFOREACH(file)

LIST(APPEND MEDITPLUGINS_SOURCES
  usertools/moousertools.c
  usertools/moousertools.h
  usertools/moousertools-prefs.c
  usertools/moousertools-prefs.h
  usertools/moocommand.c
  usertools/moocommand.h
  usertools/moocommand-private.h
  usertools/moocommand-lua.c
  usertools/moocommand-lua.h
  usertools/moocommanddisplay.c
  usertools/moocommanddisplay.h
  usertools/moooutputfilterregex.c
  usertools/moooutputfilterregex.h
  usertools/mookeyfile.c
  usertools/mookeyfile.h
)

IF(NOT WIN32)
  LIST(APPEND MEDITPLUGINS_SOURCES
    usertools/moocommand-sh.c
    usertools/moocommand-sh.h
  )
ENDIF(NOT WIN32)

MOO_GEN_GXML(meditplugins
  usertools/glade/mooedittools-exe.glade
  usertools/glade/mooedittools-lua.glade
  usertools/glade/moousertools.glade
)

MOO_GEN_ENUMS(meditplugins moousertools usertools/moousertools-enums-in.py usertools/moousertools-enums)

# -%- strip:true -%-
