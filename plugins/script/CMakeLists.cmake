LIST(APPEND MEDITPLUGINS_SOURCES
  script/mooscript.h
  script/mooscript.c
  script/medit-api.h
  script/medit-api.cpp
  script/medit-api-impl.h
  script/medit-api-impl.cpp
)

MOO_GEN_ENUMS(meditplugins mooscript script/mooscript-enums-in.py script/mooscript-enums)

MOO_ADD_GENERATED_FILE(meditplugins
  ${CMAKE_CURRENT_BINARY_DIR}/script/medit-api.stamp
  "${CMAKE_CURRENT_SOURCE_DIR}/script/medit-api.h;${CMAKE_CURRENT_SOURCE_DIR}/script/medit-api-impl.cpp"
  COMMAND ${PYTHON_EXECUTABLE} genapi.py --defs=medit.defs --mode=cpp --output-decl=medit-api.h.tmp --output-impl=medit-api-impl.cpp.tmp
  COMMAND ${CMAKE_COMMAND} -E copy_if_different medit-api.h.tmp medit-api.h
  COMMAND ${CMAKE_COMMAND} -E copy_if_different medit-api-impl.cpp.tmp medit-api-impl.cpp
  COMMAND ${CMAKE_COMMAND} -E remove medit-api.h.tmp medit-api-impl.cpp.tmp
  COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/script/medit-api.stamp
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/script
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/script/genapi.py ${CMAKE_CURRENT_SOURCE_DIR}/script/medit.defs
)

# -%- strip:true -%-
