#pragma once

#include "mooedit/mooeditview.h"
#include "mooedit/mooedittypes.h"

#ifdef __cplusplus

namespace moo {

MOO_DEFINE_GOBJ_TYPE(MooEditView, MooTextView, moo_edit_view_get_type())
//template<>                                                                                  
//struct gobjinfo<MooEditView>
//{                                                                                           
//using object_type = MooEditView;
//using parent_type = MooTextView;
//static GType object_g_type() { return moo_edit_view_get_type(); }
//static GType parent_g_type() { return gobjinfo<MooTextView>::object_g_type(); }
//};                                                                                          
//
//template<>                                                                                  
//struct gobj_is_subclass<MooEditView, MooEditView>
//{                                                                                           
//static const bool value = true;                                                         
//static MooEditView* down_cast(MooEditView* o) { return o; }
//};                                                                                          
//
//template<typename Super>                                                                    
//struct gobj_is_subclass<MooEditView, Super>
//{                                                                                           
//static const bool value = true;                                                         
//static Super* down_cast(MooEditView *o)
//{                                                                                       
//static_assert(gobj_is_subclass<MooEditView, Super>::value,
//              "In " __FUNCTION__ ": Super is not a superclass of MooEditView");
//MooTextView* p = reinterpret_cast<MooTextView*>(o);
//    Super* s = gobj_is_subclass<MooTextView, Super>::down_cast(p);
//    return s;                                                                           
//}                                                                                       
//};

template<>
class gobjref<MooEditView> : public gobjref_parent<MooEditView>
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
class gobjptr<MooEditView> : public gobjptr_impl<MooEditView>
{
public:
    MOO_DEFINE_GOBJPTR_METHODS(MooEditView);

    static gobjptr  _create (MooEditRef doc);
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
