SET(moopython_pluginsdir ${MOO_PLUGINS_DIR})
SET(plugins_libdir ${MOO_PLUGINS_DIR}/lib)

SET(plugins
  terminal.py
  python.py
  pycmd.py
)

SET(INI_IN_IN_FILES
  terminal.ini.in.in
  python.ini.in.in
  pycmd.ini.in.in
)

IF(NOT WIN32)
  LIST(APPEND plugins terminal.py)
  LIST(APPEND INI_IN_IN_FILES terminal.ini.in.in)
ENDIF(NOT WIN32)

MOO_TRANSFORM_INI_FILES(INI_FILES ${INI_IN_IN_FILES})
ADD_CUSTOM_TARGET(python-plugins-ini DEPENDS ${INI_IN_IN_FILES})

INSTALL(FILES ${INI_FILES} ${plugins} DESTINATION ${moopython_pluginsdir})
INSTALL(FILES lib/pyconsole.py lib/insert_date_and_time.py DESTINATION ${plugins_libdir})
INSTALL(FILES medit/__init__.py medit/runpython.py DESTINATION ${plugins_libdir}/medit)
