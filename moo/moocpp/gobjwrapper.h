/*
 *   moocpp/gobjwrapper.h
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

#pragma once

#ifdef __cplusplus

#include <moocpp/gobjinfo.h>
#include <moocpp/gobjref.h>
#include <moocpp/grefptr.h>
#include <mooutils/mooutils-messages.h>

namespace moo {

///////////////////////////////////////////////////////////////////////////////////////////
//
// gobj_wrapper
//

class gobj_wrapper_base
{
protected:
    static gobj_wrapper_base& get(gobj_ref<GObject> g);

    gobj_wrapper_base(gobj_ref<GObject> g);
    virtual ~gobj_wrapper_base();

    MOO_DISABLE_COPY_OPS(gobj_wrapper_base);

private:
    static GQuark qdata_key;
    static void free_qdata(gpointer d);
};

template<typename CObject, typename CppObject>
class gobj_wrapper : public gobj_ref<CObject>, public gobj_wrapper_base
{
    using obj_ref_type = gobj_ref<CObject>;
    using obj_ptr_type = gobj_ptr<CObject>;

public:
    template<typename ...Args>
    static gref_ptr<CppObject> create(Args&& ...args)
    {
        return create<CppObject>(std::forward<Args>(args)...);
    }

    template<typename Subclass, typename ...Args>
    static gref_ptr<Subclass> create(Args&& ...args)
    {
        obj_ptr_type g = create_gobj<CObject>();
        gobj_wrapper_data d{g.gobj()};
        auto o = gref_ptr<Subclass>::create(d, std::forward<Args>(args)...);
        // Now g and o share the ref count, which is currently owned by the g pointer; steal it.
        g.release();
        return o;
    }

    static CppObject& get(obj_ref_type gobj)
    {
        gobj_wrapper_base& b = gobj_wrapper_base::get(gobj);
        return static_cast<CppObject&>(b);
    }

    void ref()
    {
        g_object_ref (gobj());
    }

    void unref()
    {
        g_object_unref (gobj());
    }

    MOO_DISABLE_COPY_OPS(gobj_wrapper);

protected:
    struct gobj_wrapper_data { CObject* g; };

    gobj_wrapper(gobj_wrapper_data& d)
        : obj_ref_type(*d.g)
        , gobj_wrapper_base(obj_ref_type(*d.g))
    {
    }

    virtual ~gobj_wrapper()
    {
    }
};

} // namespace moo

#endif // __cplusplus
