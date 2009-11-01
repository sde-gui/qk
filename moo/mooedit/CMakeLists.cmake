INSTALL(FILES mooedit/medit.lua DESTINATION ${MOO_DATA_DIR}/lua)

SET(completion_sources
  moocompletionsimple.c
  moocompletionsimple.h
  mootextcompletion.c
  mootextcompletion.h
  mootextpopup.c
  mootextpopup.h
)

SET(MOOEDIT_SOURCES
  ${completion_sources}
  moocmdview.c
  moocmdview.h
  mooedit-accels.h
  mooeditaction.c
  mooeditaction-factory.c
  mooeditaction-factory.h
  mooeditaction.h
  mooedit-bookmarks.c
  mooedit-bookmarks.h
  mooedit.c
  mooeditconfig.c
  mooeditconfig.h
  mooeditdialogs.c
  mooeditdialogs.h
  mooedit-enums.c
  mooedit-enums.h
  mooedit-fileops.c
  mooedit-fileops.h
  mooeditfiltersettings.c
  mooeditfiltersettings.h
  mooedit.h
  mooedit-lua-api.c
  mooedit-lua.c
  mooedit-lua.h
  mooeditor.c
  mooeditor.h
  mooeditor-private.h
  mooeditprefs.c
  mooeditprefs.h
  mooeditprefspage.c
  mooedit-private.h
  mooedit-tests.h
  mooeditwindow.c
  mooeditwindow.h
  moofold.c
  moofold.h
  mooindenter.c
  mooindenter.h
  moolang.c
  moolang.h
  moolangmgr.c
  moolangmgr.h
  moolangmgr-private.h
  moolang-private.h
  moolinebuffer.c
  moolinebuffer.h
  moolinemark.c
  moolinemark.h
  moolineview.c
  moolineview.h
  moooutputfilter.c
  moooutputfilter.h
  mooplugin.c
  mooplugin.h
  mooplugin-loader.c
  mooplugin-loader.h
  mooplugin-macro.h
  mootextbox.c
  mootextbox.h
  mootextbtree.c
  mootextbtree.h
  mootextbuffer.c
  mootextbuffer.h
  mootextfind.c
  mootextfind.h
  mootextiter.h
  mootext-private.h
  mootextsearch.c
  mootextsearch.h
  mootextsearch-private.h
  mootextstylescheme.c
  mootextstylescheme.h
  mootextview.c
  mootextview.h
  mootextview-input.c
  mootextview-private.h
)

SET(printing_sources
  mooprintpreview.h
  mooprintpreview.c
  mootextprint.c
  mootextprint-private.h
  mootextprint.h
)

LIST(APPEND MOOEDIT_SOURCES ${printing_sources})

MOO_GEN_GXML(mooedit
  glade/mooprintpreview.glade
  glade/moopluginprefs.glade
  glade/mooeditprefs.glade
  glade/mooeditprogress.glade
  glade/mooeditsavemult.glade
  glade/mooprint.glade
  glade/mootextfind.glade
  glade/mootextfind-prompt.glade
  glade/mootextgotoline.glade
  glade/mooquicksearch.glade
  glade/moostatusbar.glade
)

MOO_GEN_ENUMS(mooedit mooedit mooedit/mooedit-enums-in.py mooedit/mooedit-enums)

MOO_GEN_UIXML(mooedit medit.xml mooedit.xml)

# AM_CFLAGS_ = -Igtksourceview $(MOO_CFLAGS) $(MOO_WIN32_CFLAGS)
# AM_CXXFLAGS_ = -Igtksourceview $(MOO_CXXFLAGS) $(MOO_WIN32_CFLAGS)

INCLUDE(mooedit/gtksourceview/CMakelists.cmake)
INCLUDE(mooedit/langs/CMakelists.cmake)

MOO_ADD_MOO_CODE_MODULE(mooedit)

# -%- strip:true -%-
