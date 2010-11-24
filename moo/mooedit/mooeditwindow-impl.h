#ifndef MOO_EDIT_WINDOW_IMPL_H
#define MOO_EDIT_WINDOW_IMPL_H

#include "mooedit/mooeditwindow.h"

G_BEGIN_DECLS

void             _moo_edit_window_insert_doc    (MooEditWindow  *window,
                                                 MooEdit        *doc,
                                                 int             position);
void             _moo_edit_window_remove_doc    (MooEditWindow  *window,
                                                 MooEdit        *doc,
                                                 gboolean        destroy);
int              _moo_edit_window_get_doc_no    (MooEditWindow  *window,
                                                 MooEdit        *doc);
void             _moo_edit_window_update_title  (void);
void             _moo_edit_window_set_use_tabs  (void);

G_END_DECLS

#endif /* MOO_EDIT_WINDOW_IMPL_H */
