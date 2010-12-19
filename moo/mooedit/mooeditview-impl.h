#ifndef MOO_EDIT_VIEW_IMPL_H
#define MOO_EDIT_VIEW_IMPL_H

#include "mooedit/mooeditview.h"

G_BEGIN_DECLS

void            _moo_edit_view_set_doc                  (MooEditView    *view,
                                                         MooEdit        *doc);
void            _moo_edit_view_apply_config             (MooEditView    *view);

MooEditState    _moo_edit_view_get_state                (MooEditView    *view);
void            _moo_edit_view_set_progress_text        (MooEditView    *view,
                                                         const char     *text);
void            _moo_edit_view_set_state                (MooEditView    *view,
                                                         MooEditState    state,
                                                         const char     *text,
                                                         GDestroyNotify  cancel,
                                                         gpointer        data);

void            _moo_edit_view_ui_set_line_wrap         (MooEditView    *view,
                                                         gboolean        enabled);
void            _moo_edit_view_ui_set_show_line_numbers (MooEditView    *view,
                                                         gboolean        show);

void            _moo_edit_view_do_popup                 (MooEditView    *view,
                                                         GdkEventButton *event);

G_END_DECLS

#endif /* MOO_EDIT_VIEW_IMPL_H */
