SET(test_sources
  moo-test-utils.c
  moo-test-utils.h
  moo-test-macros.h
)

SET(win32_sources
  mooutils-win32.c
)

SET(thread_sources
  mooutils-thread.c
  mooutils-thread.h
)

SET(MOOUTILS_SOURCES
  ${thread_sources}
  mdhistorymgr.c
  mdhistorymgr.h
  moo-environ.h
  mooaccel.c
  mooaccel.h
  mooaccelbutton.c
  mooaccelbutton.h
  mooaccelprefs.c
  mooaccelprefs.h
  mooaction-private.h
  mooaction.c
  mooaction.h
  mooactionbase-private.h
  mooactionbase.c
  mooactionbase.h
  mooactioncollection.c
  mooactioncollection.h
  mooactionfactory.c
  mooactionfactory.h
  mooactiongroup.c
  mooactiongroup.h
  mooapp-ipc.c
  mooapp-ipc.h
  mooappinput.c
  mooappinput.h
  mooatom.h
  moobigpaned.c
  moobigpaned.h
  mooclosure.c
  mooclosure.h
  moocombo.c
  moocombo.h
  moocompat.c
  moocompat.h
  moodialogs.c
  moodialogs.h
  mooeditops.c
  mooeditops.h
  mooencodings-data.h
  mooencodings.c
  mooencodings.h
  mooentry.c
  mooentry.h
  moofiledialog.c
  moofiledialog.h
  moofileicon.c
  moofileicon.h
  moofilewatch.c
  moofilewatch.h
  moofiltermgr.c
  moofiltermgr.h
  moofontsel.c
  moofontsel.h
  mooglade.c
  mooglade.h
  moohelp.c
  moohelp.h
  moohistorycombo.c
  moohistorycombo.h
  moohistorylist.c
  moohistorylist.h
  mooi18n.c
  mooi18n.h
  moolist.h
  moomarkup.c
  moomarkup.h
  moomenu.c
  moomenu.h
  moomenuaction.c
  moomenuaction.h
  moomenumgr.c
  moomenumgr.h
  moomenutoolbutton.c
  moomenutoolbutton.h
  moo-mime.c
  moo-mime.h
  moonotebook.c
  moonotebook.h
  moopane.c
  moopane.h
  moopaned.c
  moopaned.h
  mooprefs.c
  mooprefs.h
  mooprefsdialog.c
  mooprefsdialog.h
  mooprefspage.c
  mooprefspage.h
  moopython.h
  moospawn.c
  moospawn.h
  moostock.c
  moostock.h
  mootype-macros.h
  moouixml.c
  moouixml.h
  mooundo.c
  mooundo.h
  mooutils-debug.h
  mooutils-enums.c
  mooutils-enums.h
  mooutils-file.c
  mooutils-file.h
  mooutils-file-private.h
  mooutils-fli.c
  mooutils-fs.c
  mooutils-fs.h
  mooutils-gobject-private.h
  mooutils-gobject.c
  mooutils-gobject.h
  mooutils-mem.h
  mooutils-misc.c
  mooutils-misc.h
  mooutils-tests.h
  mooutils-treeview.c
  mooutils-treeview.h
  moowindow.c
  moowindow.h
  stock-file-24.h
  stock-file-selector-24.h
  stock-select-all-16.h
  stock-select-all-24.h
  stock-terminal-24.h
)

IF(WIN32)
  LIST(APPEND MOOUTILS_SOURCES ${win32_sources})
ENDIF(WIN32)
IF(MOO_ENABLE_UNIT_TESTS)
  LIST(APPEND MOOUTILS_SOURCES ${test_sources})
ENDIF(MOO_ENABLE_UNIT_TESTS)

MOO_GEN_GXML(
  glade/mooaccelprefs.glade
  glade/mooaccelbutton.glade
  glade/moologwindow.glade
)

MACRO(MOO_GEN_PIXMAP_FILE output)
  SET(output_full ${CMAKE_CURRENT_BINARY_DIR}/${output})
  SET(stamp ${output_full}.stamp)
  SET(pixmaps)
  SET(pixbufargs)

  SET(name)
  FOREACH(_arg ${ARGN})
    IF(NOT name)
      SET(name ${_arg})
    ELSE(NOT name)
      SET(icon ${_arg})
      LIST(APPEND pixbufargs ${name} ${CMAKE_SOURCE_DIR}/moo/mooutils/pixmaps/${icon})
      LIST(APPEND pixmaps ${CMAKE_SOURCE_DIR}/moo/mooutils/pixmaps/${icon})
      SET(name)
    ENDIF(NOT name)
  ENDFOREACH(_arg)

  SET(genpixbuf_csource ${CMAKE_CURRENT_BINARY_DIR}/genpixbuf-source-${output}.cmake)
  CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/moo/mooutils/genpixbufcsource.cmake.in ${genpixbuf_csource} @ONLY)

  MOO_ADD_GENERATED_FILE(
    ${stamp} ${output_full}
    COMMAND ${CMAKE_COMMAND} -P ${genpixbuf_csource}
    DEPENDS ${pixmaps} ${genpixbuf_csource}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )
ENDMACRO(MOO_GEN_PIXMAP_FILE)

MOO_GEN_PIXMAP_FILE(stock-moo.h
  MOO_HIDE_ICON hide.png
  MOO_CLOSE_ICON close.png
  MOO_STICKY_ICON sticky.png
  MOO_DETACH_ICON detach.png
  MOO_ATTACH_ICON attach.png
  MOO_KEEP_ON_TOP_ICON keepontop.png
)

MOO_GEN_PIXMAP_FILE(stock-medit.h
  MEDIT_ICON medit.png
)

SET(moo_utils_enum_headers
  moofiledialog.h
  moofilewatch.h
  mooprefs.h
  moouixml.h
  mooutils-misc.h
)

MOO_GEN_PIXMAP_FILE(moofileicon-symlink.h
  SYMLINK_ARROW symlink.png
  SYMLINK_ARROW_SMALL symlink-small.png
)

IF(GLIB_VERSION VERSION_LESS "2.16.0")
  ADD_SUBDIRECTORY(newgtk)
ENDIF(GLIB_VERSION VERSION_LESS "2.16.0")

MOO_ADD_MOO_CODE_MODULE(mooutils)

ADD_SUBDIRECTORY(xdgmime)

# -%- strip:true -%-
