#ifndef MOO_EDITOR_IMPL_H
#define MOO_EDITOR_IMPL_H

#ifndef MOOEDIT_COMPILATION
#error "This file may not be used directly"
#endif

#include "mooedit/mooeditor.h"
#include "mooutils/mdhistorymgr.h"
#include "mooutils/moofilewatch.h"

G_BEGIN_DECLS

MdHistoryMgr    *_moo_editor_get_history_mgr    (MooEditor      *editor);

void             _moo_editor_set_focused_doc    (MooEditor      *editor,
                                                 MooEdit        *doc);
void             _moo_editor_unset_focused_doc  (MooEditor      *editor,
                                                 MooEdit        *doc);

void             _moo_editor_move_doc           (MooEditor      *editor,
                                                 MooEdit        *doc,
                                                 MooEditWindow  *dest,
                                                 gboolean        focus);

MooFileWatch    *_moo_editor_get_file_watch     (MooEditor      *editor);
void             _moo_editor_reload             (MooEditor      *editor,
                                                 MooEdit        *doc,
                                                 const char     *encoding,
                                                 GError        **error);
gboolean         _moo_editor_save               (MooEditor      *editor,
                                                 MooEdit        *doc,
                                                 GError        **error);
gboolean         _moo_editor_save_as            (MooEditor      *editor,
                                                 MooEdit        *doc,
                                                 const char     *filename,
                                                 const char     *encoding,
                                                 GError        **error);

G_END_DECLS

#endif /* MOO_EDITOR_IMPL_H */
