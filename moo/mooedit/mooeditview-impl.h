#pragma once

#include "mooedit/mooeditview.h"
#include "mooedit/mooedittypes.h"

#ifdef __cplusplus

template<>
class moo::gobj_ref<MooEditView> : public moo::gobj_ref_parent<MooEditView>
{
public:
    MOO_DEFINE_GOBJREF_METHODS(MooEditView);

    void            _unset_doc              ();
    void            _set_tab                (MooEditTab*    tab);

    GtkTextMark*    _get_fake_cursor_mark   ();

    void            _apply_config           ();

    static gobj_ptr<MooEditView> _create (Edit doc);

    MooEditViewPrivate&         get_priv()          { return *gobj()->priv; }
    const MooEditViewPrivate&   get_priv() const    { return *gobj()->priv; }
};

MOO_REGISTER_CUSTOM_GOBJ_TYPE(MooEditView);

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
