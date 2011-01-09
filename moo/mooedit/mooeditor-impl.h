#ifndef MOO_EDITOR_IMPL_H
#define MOO_EDITOR_IMPL_H

#ifndef MOOEDIT_COMPILATION
#error "This file may not be used directly"
#endif

#include "mooedit/mooeditor.h"
#include "mooutils/moohistorymgr.h"
#include "mooutils/moofilewatch.h"

G_BEGIN_DECLS

MooHistoryMgr   *_moo_editor_get_history_mgr    (MooEditor      *editor);

void             _moo_editor_set_focused_doc    (MooEditor      *editor,
                                                 MooEdit        *doc);
void             _moo_editor_unset_focused_doc  (MooEditor      *editor,
                                                 MooEdit        *doc);

void             _moo_editor_move_doc           (MooEditor      *editor,
                                                 MooEdit        *doc,
                                                 MooEditWindow  *dest,
                                                 MooEditView    *dest_view,
                                                 gboolean        focus);

MooFileWatch    *_moo_editor_get_file_watch     (MooEditor      *editor);

void             _moo_editor_apply_prefs        (MooEditor      *editor);

G_END_DECLS

#endif /* MOO_EDITOR_IMPL_H */
