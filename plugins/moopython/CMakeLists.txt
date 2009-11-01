INCLUDE(plugins/CMakeLists.cmake)

SET(MOOMOD_SOURCES
  moo-mod.c
  moopython-api.h
  moopython-utils.h
  moopython-utils.c
  moopython-loader.h
  moopython-loader.c
)

SET(PYMOO_SOURCES
  moopython-mod.c
  moopython-pygtkmod.h
  moopython-api.h
  moopython-loader.h
  moopython-loader.c
  moopython-utils.h
  moopython-utils.c
)

SET(MOOPYTHON_SOURCES
  moopython-builtin.h
  moopython-builtin.c
  moopython-api.h
  moopython-loader.h
  moopython-loader.c
  moopython-utils.h
  moopython-utils.c
)

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}/pygtk
  ${CMAKE_CURRENT_BINARY_DIR}/pygtk
  ${PYTHON_INCLUDE_PATH}
  ${PYGTK_INCLUDE_DIRS}
)

IF(WIN32 AND MOO_BUILD_SHARED_LIB)
  ADD_DEFINITIONS(-DMOO_PYTHON_MODULE_DLL_NAME=\"pymoo.dll\")
ENDIF(WIN32 AND MOO_BUILD_SHARED_LIB)

INCLUDE(pygtk/CMakeLists.cmake)

IF(MOO_BUILD_SHARED_LIB)
  MOO_ADD_LIBRARY(moomod MODULE ${MOOPYGTK_SOURCES})
  MOO_ADD_LIBRARY(pymoo MODULE)
  TARGET_LINK_LIBRARIES(moomod moo ${PYGTK_LIBRARIES} ${PYTHON_LIBRARIES})
ELSE(MOO_BUILD_SHARED_LIB)
  MOO_ADD_LIBRARY(moopython ${MOOMOD_SOURCES} ${MOOPYGTK_SOURCES})
ENDIF(MOO_BUILD_SHARED_LIB)

# -%- strip:true -%-
