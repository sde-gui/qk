#pragma once

#include "mooedit/mooeditview.h"
#include "mooedit/mooedittypes.h"

#ifdef __cplusplus

MOO_DEFINE_GOBJ_TYPE(MooEditView, MooTextView, moo_edit_view_get_type())

namespace moo {

template<>
class gobj_ref<MooEditView> : public gobj_ref_parent<MooEditView>
{
public:
    MOO_DEFINE_GOBJREF_METHODS(MooEditView);

    void            _unset_doc              ();
    void            _set_tab                (MooEditTab*    tab);

    GtkTextMark*    _get_fake_cursor_mark   ();

    void            _apply_config           ();

    MooEditViewPrivate&         get_priv()          { return *gobj()->priv; }
    const MooEditViewPrivate&   get_priv() const    { return *gobj()->priv; }
};

template<>
class gobj_ptr<MooEditView> : public gobj_ptr_impl<MooEditView>
{
public:
    MOO_DEFINE_GOBJPTR_METHODS(MooEditView);

    static gobj_ptr _create (Edit doc);
};

} // namespace moo

#endif // __cplusplus

G_BEGIN_DECLS

void            _moo_edit_view_apply_prefs              (MooEditView    *view);

void            _moo_edit_view_ui_set_line_wrap         (MooEditView    *view,
                                                         gboolean        enabled);
void            _moo_edit_view_ui_set_show_line_numbers (MooEditView    *view,
                                                         gboolean        show);

void            _moo_edit_view_do_popup                 (MooEditView    *view,
                                                         GdkEventButton *event);

G_END_DECLS
