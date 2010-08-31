bin_PROGRAMS = medit

AM_CPPFLAGS = $(MOO_CPPFLAGS) -I$(top_srcdir) -I$(top_srcdir)/moo -I$(top_srcdir)/doc
AM_CFLAGS = $(MOO_CFLAGS)
AM_CXXFLAGS = $(MOO_CXXFLAGS)

# if MOO_ENABLE_UNIT_TESTS
# AM_CFLAGS += -DSRCDIR="${CMAKE_SOURCE_DIR}/medit")
# endif

medit_SOURCES =		\
	mem-debug.h	\
	run-tests.h	\
	parse.h		\
	main.c

nodist_EXTRA_medit_SOURCES =	\
	dummy.cpp

medit_LDADD = ../plugins/libmooeditplugins.la ../moo/libmoo.la $(MOO_LIBS)
medit_LDFLAGS =
if MOO_OS_WIN32
medit_LDFLAGS += -mwindows
endif

# EXTRA_DIST += medit.desktop.in
# desktopdir = $(datadir)/applications
# desktop_DATA = medit.desktop.in

# MOO_WIN32_RESOURCE(medit.rc.in medit MEDIT_SOURCES medit_libs)

# if !MOO_ENABLE_UNIT_TESTS
# MEDIT_WIN32 = Win32
# endif
#   SET(MEDIT_WIN32 WIN32)
# ENDIF(NOT MOO_ENABLE_UNIT_TESTS)

# ADD_EXECUTABLE(medit ${MEDIT_WIN32} ${MEDIT_SOURCES})
# TARGET_LINK_LIBRARIES(medit ${medit_libs} ${MOO_DEP_LIBS})
# INSTALL(TARGETS medit DESTINATION ${BINDIR})

# IF(MOO_ENABLE_PYTHON)
#   ADD_DEPENDENCIES(medit python-plugins-ini)
# ENDIF(MOO_ENABLE_PYTHON)

# FIND_PACKAGE(MooDesktopFile)
# MOO_INSTALL_DESKTOP_FILE(medit.desktop.in)
#
# IF(MOO_OS_UNIX)
#   SET(PIXMAPS_DIR "${DATADIR}/pixmaps")
#   SET(ICONTHEME_DIR "${DATADIR}/icons/hicolor/48x48/apps")
#   SET(MEDIT_ICONS "${CMAKE_SOURCE_DIR}/moo/mooutils/pixmaps/medit.png")
#   INSTALL(FILES ${MEDIT_ICONS} DESTINATION "${PIXMAPS_DIR}")
#   INSTALL(FILES ${MEDIT_ICONS} DESTINATION "${ICONTHEME_DIR}")
# ENDIF(MOO_OS_UNIX)
