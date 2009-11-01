SET(MOOFILEVIEW_SOURCES
  moobookmarkmgr.c
  moobookmarkmgr.h
  moobookmarkview.c
  moobookmarkview.h
  moofile.c
  moofile.h
  moofile-private.h
  moofileentry.c
  moofileentry.h
  moofilesystem.c
  moofilesystem.h
  moofileview.c
  moofileview.h
  moofileview-accels.h
  moofileview-aux.h
  moofileview-dialogs.c
  moofileview-dialogs.h
  moofileview-impl.h
  moofileview-private.h
  moofileview-tools.c
  moofileview-tools.h
  moofolder-private.h
  moofolder.c
  moofolder.h
  moofoldermodel.c
  moofoldermodel.h
  moofoldermodel-private.h
  mooiconview.c
  mooiconview.h
  mootreeview.c
  mootreeview.h
)

MOO_GEN_GXML(moofileview
  glade/moofileview-drop.glade
  glade/moofileprops.glade
  glade/moocreatefolder.glade
  glade/moobookmark-editor.glade
)

MOO_GEN_UIXML(moofileview moofileview.xml)

MOO_ADD_MOO_CODE_MODULE(moofileview)

# -%- strip:true -%-
