#include "moocpp-gtk.h"

namespace moo {

RefCount::operator int() const
{
    return g_atomic_int_get(&m_count);
}

void RefCount::ref()
{
    g_atomic_int_inc(&m_count);
}

bool RefCount::unref()
{
    mooAssert(int(*this) > 0);
    return g_atomic_int_dec_and_test(&m_count) != 0;
}

#ifdef DEBUG

namespace _test {

#define MOO_CHECK_ERR 0

inline void func()
{
    GtkTreeView *tv;
    GtkWidget *w;
    SharedPtr<Gtk::TreeView> sptv;
    SharedPtr<Gtk::Widget> spw;
    WeakPtr<Gtk::TreeView> wptv;
    WeakPtr<Gtk::Widget> wpw;

    wptv = sptv;
    wpw = sptv;
    wpw = wptv;
#if MOO_CHECK_ERR
    sptv = wptv;
    wptv = spw;
    wptv = wpw;
#endif

    spw = sptv;
#if MOO_CHECK_ERR
    sptv = spw;
#endif

    sptv = Gtk::TreeView::get(tv);
    spw = Gtk::TreeView::get(tv);
    spw = Gtk::Widget::get(tv);
    spw = Gtk::Widget::get(w);
#if MOO_CHECK_ERR
    Gtk::TreeView::get(w);
    sptv = Gtk::Widget::get(tv);
    sptv = Gtk::Widget::get(w);
#endif

    w = down_cast<GtkWidget>(w);
    w = runtime_cast<GtkWidget>(w);
    w = down_cast<GtkWidget>(tv);
    w = runtime_cast<GtkWidget>(tv);
    tv = runtime_cast<GtkTreeView>(w);
#if MOO_CHECK_ERR
    tv = down_cast<GtkTreeView>(w);
#endif
}

} // namespace _test

#endif // DEBUG

} // namespace moo
// -%- strip:true -%-
