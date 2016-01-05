/*
 *   moocpp/gobjptrtypes-gtk.cpp
 *
 *   Copyright (C) 2004-2016 by Yevgen Muntyan <emuntyan@users.sourceforge.net>
 *
 *   This file is part of medit.  medit is free software; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU Lesser General Public License as published by the
 *   Free Software Foundation; either version 2.1 of the License,
 *   or (at your option) any later version.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with medit.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "moocpp/gobjptrtypes-gtk.h"

struct A {};

namespace moo {

//MOO_DEFINE_GOBJ_TYPE(GtkToolbar, GtkWidget, GTK_TYPE_TOOLBAR)

namespace _test {

void test()
{
    //create_cpp_gobj<A>();
    //init_cpp_gobj(a);

    GtkObject* xyz = nullptr;
    init_cpp_gobj(xyz);

    //create_cpp_gobj<GtkToolbar>(GTK_TYPE_TOOLBAR);

    {
        gobj_ptr<GtkObject> p;
        gobj_ref<GtkObject>& r = *p;
        GtkObject* o1 = r.gobj();
        GtkObject* o2 = p->gobj();
        g_assert(o1 == o2);
        GObject* o = p.gobj<GObject>();
        g_assert(o == nullptr);
        GtkObject* x = p.gobj<GtkObject>();
        GObject* y = p.gobj<GObject>();
        g_assert((void*) x == (void*) y);
        const GObject* c1 = p;
        const GtkObject* c2 = p;
        g_assert((void*) c1 == (void*) c2);
    }

    {
        gobj_ptr<GtkWidget> p = wrap_new(gtk_widget_new(0, "blah", nullptr, nullptr));
        gobj_ref<GtkWidget>& r = *p;
        GtkWidget* o1 = r.gobj();
        GtkWidget* o2 = p->gobj();
        g_assert(o1 == o2);
        GtkWidget* x = p.gobj<GtkWidget>();
        GtkWidget* y = p.gobj();
        GtkObject* z = p.gobj<GtkObject>();
        GObject* t = p.gobj<GObject>();
        g_assert((void*) x == (void*) y);
        g_assert((void*) z == (void*) t);
        const GObject* c1 = p;
        const GtkObject* c2 = p;
        const GtkWidget* c3 = p;
        g_assert((void*) c1 == (void*) c2);
        g_assert((void*) c1 == (void*) c3);

        gobj_ref<GtkWidget> or(*p.gobj());
        or.freeze_notify();
        p->freeze_notify();

        gobj_raw_ptr<GtkWidget> rp = p.gobj();
    }
}

} // namespace _test
} // namespace moo
