#include "moocpp.h"

// #include "moocpp-gtk.h"
//
// namespace moo {
//
// #ifdef DEBUG
//
// namespace _test {
//
// #define MOO_CHECK_ERR 0
//
// inline void func()
// {
//     GtkTreeView *tv;
//     GtkWidget *w;
//     SharedPtr<Gtk::TreeView> sptv;
//     SharedPtr<Gtk::Widget> spw;
//     WeakPtr<Gtk::TreeView> wptv;
//     WeakPtr<Gtk::Widget> wpw;
//
//     wptv = sptv;
//     wpw = sptv;
//     wpw = wptv;
// #if MOO_CHECK_ERR
//     sptv = wptv;
//     wptv = spw;
//     wptv = wpw;
// #endif
//
//     spw = sptv;
// #if MOO_CHECK_ERR
//     sptv = spw;
// #endif
//
//     sptv = Gtk::TreeView::get(tv);
//     spw = Gtk::TreeView::get(tv);
//     spw = Gtk::Widget::get(tv);
//     spw = Gtk::Widget::get(w);
// #if MOO_CHECK_ERR
//     Gtk::TreeView::get(w);
//     sptv = Gtk::Widget::get(tv);
//     sptv = Gtk::Widget::get(w);
// #endif
//
//     w = down_cast<GtkWidget>(w);
//     w = runtime_cast<GtkWidget>(w);
//     w = down_cast<GtkWidget>(tv);
//     w = runtime_cast<GtkWidget>(tv);
//     tv = runtime_cast<GtkTreeView>(w);
// #if MOO_CHECK_ERR
//     tv = down_cast<GtkTreeView>(w);
// #endif
// }
//
// } // namespace _test
//
// #endif // DEBUG
//
// } // namespace moo
// // -%- strip:true -%-
