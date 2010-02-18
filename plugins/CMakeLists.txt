INCLUDE_DIRECTORIES(${MOO_SOURCE_DIR}/doc ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

SET(MEDITPLUGINS_SOURCES)

###########################################################################
# ctags
#

SET(ctags_sources
  ctags/readtags.c
  ctags/readtags.h
  ctags/readtags-mangle.h
  ctags/ctags-plugin.c
  ctags/ctags-doc.c
  ctags/ctags-doc.h
  ctags/ctags-view.c
  ctags/ctags-view.h
)

IF(MOO_BUILD_CTAGS)
  LIST(APPEND MEDITPLUGINS_SOURCES ${ctags_sources})
ENDIF(MOO_BUILD_CTAGS)

###########################################################################
# fileselector
#

MOO_GEN_GXML(
  glade/moofileselector-prefs.glade
  glade/moofileselector.glade
)

LIST(APPEND MEDITPLUGINS_SOURCES
  moofileselector-prefs.c
  moofileselector.c
  moofileselector.h
)

###########################################################################
# user tools
#

ADD_SUBDIRECTORY(usertools)

# ###########################################################################
# # script
# #
#
# INCLUDE(script/CMakeLists.cmake)

###########################################################################
# libmooeditplugins
#

LIST(APPEND MEDITPLUGINS_SOURCES
  mooeditplugins.h
  mooeditplugins.c
  moofilelist.c
  moofind.c
)

MOO_GEN_GXML(
  glade/moofind.glade
  glade/moogrep.glade
)

MOO_ADD_LIBRARY(meditplugins STATIC)

# ADD_SUBDIRECTORY(moopython)

# -%- strip:true -%-
