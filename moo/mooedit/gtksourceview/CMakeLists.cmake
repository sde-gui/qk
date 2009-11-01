SET(GTK_SOURCE_VIEW_UPSTREAM_SOURCES
  gtksourcecontextengine.c
  gtksourcecontextengine.h
  gtksourceengine.c
  gtksourceengine.h
  gtksourceiter.c
  gtksourceiter.h
  gtksourcelanguage-parser-1.c
  gtksourcelanguage-parser-2.c
  gtksourcelanguage-private.h
  gtksourcelanguage.c
  gtksourcelanguage.h
  gtksourcelanguagemanager.c
  gtksourcelanguagemanager.h
  gtksourcestyle-private.h
  gtksourcestyle.c
  gtksourcestyle.h
  gtksourcestylescheme.c
  gtksourcestylescheme.h
  gtksourcestyleschememanager.c
  gtksourcestyleschememanager.h
  gtksourceview-utils.c
  gtksourceview-utils.h
  gtktextregion.c
  gtktextregion.h
)

SET(GTK_SOURCE_VIEW_BUILT_SOURCES)
FOREACH(_moo_f ${GTK_SOURCE_VIEW_UPSTREAM_SOURCES})
  STRING(REGEX REPLACE "^(.*)\\.([ch])$" "${CMAKE_CURRENT_SOURCE_DIR}/mooedit/gtksourceview/upstream/\\1.\\2" _moo_input ${_moo_f})
  STRING(REGEX REPLACE "^(.*)\\.([ch])$" "${CMAKE_CURRENT_BINARY_DIR}/mooedit/gtksourceview/\\1-mangled.\\2" _moo_output ${_moo_f})
  MOO_ADD_GENERATED_FILE(
    mooedit ${_moo_output}.stamp ${_moo_output}
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/mooedit/gtksourceview/mangle.py ${_moo_input} ${_moo_output}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/mooedit/gtksourceview/mangle.py ${_moo_input}
  )
  LIST(APPEND GTK_SOURCE_VIEW_BUILT_SOURCES ${_moo_output})
ENDFOREACH(_moo_f)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/moo/mooedit/gtksourceview ${CMAKE_CURRENT_BINARY_DIR})

SET(GTK_SOURCE_VIEW_SOURCES
  gtksourceview/gtksourceview-i18n.h
  gtksourceview/gtksourceview-marshal.h
  gtksourceview/gtksourceview-api.h
)

LIST(APPEND MOOEDIT_SOURCES ${GTK_SOURCE_VIEW_SOURCES})

# -%- strip:true -%-
