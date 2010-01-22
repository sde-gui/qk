/*
 *   moocppg-gobject.h
 *
 *   Copyright (C) 2004-2009 by Yevgen Muntyan <muntyan@tamu.edu>
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

#ifndef MOO_CPP_GOBJECT_H
#define MOO_CPP_GOBJECT_H

#include <glib-object.h>
#include <mooutils/moocpp-macros.h>
#include <mooutils/moocpp-refptr.h>
#include <mooutils/moocpp-cont.h>

namespace moo {

#define MOO_DEFINE_GOBJECT_PRIV_CPP(ImplType, CType, c_type)    \
typedef ImplType _##CType##_ImplType;                           \
                                                                \
static void c_type##_init(CType *obj)                           \
{                                                               \
    MOO_BEGIN_NO_EXCEPTIONS                                     \
    ImplType *p = G_TYPE_INSTANCE_GET_PRIVATE(obj,              \
        c_type##_get_type(), ImplType);                         \
    new (p) ImplType;                                           \
    MOO_END_NO_EXCEPTIONS                                       \
}                                                               \
                                                                \
static void NOTHROW c_type##_finalize(GObject *gobj)            \
{                                                               \
    ImplType *p = G_TYPE_INSTANCE_GET_PRIVATE(gobj,             \
        c_type##_get_type(), ImplType);                         \
    p->~ImplType();                                             \
    G_OBJECT_CLASS(c_type##_parent_class)->finalize(gobj);      \
}                                                               \
                                                                \
static ImplType * NOTHROW getPriv(CType *obj)                   \
{                                                               \
    return G_TYPE_INSTANCE_GET_PRIVATE(obj,                     \
        c_type##_get_type(), ImplType);                         \
}

#define MOO_GOBJECT_PRIV_CPP_CLASS_INIT(CType, c_type)          \
    G_OBJECT_CLASS(klass)->finalize = c_type##_finalize;        \
    g_type_class_add_private(klass, sizeof(_##CType##_ImplType))


template<typename GObjType>
class GWeakPtr
{
public:
    NOTHROW GWeakPtr(GObjType *gobj = 0) : m_p(0) { set(gobj); }
    NOTHROW ~GWeakPtr() { unset(); }

    NOTHROW GWeakPtr(const GWeakPtr &wp) : m_p(0) { set(wp.get()); }
    GWeakPtr & NOTHROW operator=(const GWeakPtr &wp) { set(wp.get()); }
    GWeakPtr & NOTHROW operator=(GObjType *gobj) { set(gobj); }

    NOTHROW operator bool () const { return m_p; }
    bool NOTHROW operator ! () const { return !m_p; }

    NOTHROW operator GObjType* () const { return get(); }
    GObjType *operator -> () const { return checkPtr(get()); }
    GObjType * NOTHROW get() const { return static_cast<GObjType*>(m_p); }

    void NOTHROW set(GObjType *gobj)
    {
        if (m_p != gobj)
        {
            unset();
            m_p = gobj;
            if (m_p)
                g_object_add_weak_pointer(G_OBJECT(m_p), &m_p);
        }
    }

    void NOTHROW unset()
    {
        if (m_p)
            g_object_remove_weak_pointer(G_OBJECT(m_p), &m_p);
        m_p = 0;
    }

private:
    gpointer m_p;
};

template<typename GObjType> class GObjectTypeHelper;

class GObjectCppProxyImpl
    : public RefCounted<GObjectCppProxyImpl>
    , public WeakRefd<GObjectCppProxyImpl>
{
private:
    GObject *m_gobj;

protected:
    GObjectCppProxyImpl(GObject *gobj);
    virtual ~GObjectCppProxyImpl();

private:
    MOO_DISABLE_COPY_AND_ASSIGN(GObjectCppProxyImpl)
    static void weakNotify(void *data, GObject *dead);

protected:
    GObject *gobj() { return m_gobj; }

    static GObjectCppProxyImpl *get(GObject *gobj);

    static void registerGType(GType type, GObjectCppProxyImpl*(*createFunc)(GObject *gobj));
    static void registerGType() {}
};

template<typename GObjTypeParent, typename GObjTypeChild>
GObjTypeParent *down_cast(GObjTypeChild *c)
{
    typedef typename GObjectTypeHelper<GObjTypeChild>::TCppClass CppClassC;
    typedef typename GObjectTypeHelper<GObjTypeParent>::TCppClass CppClassP;
    mooCheckCanCast(CppClassC, CppClassP);
    return (GObjTypeParent*) c;
}

template<typename GObjTypeChild, typename GObjTypeParent>
GObjTypeChild *runtime_cast(GObjTypeParent *p)
{
    typedef typename GObjectTypeHelper<GObjTypeChild>::TCppClass CppClass;
    if (g_type_is_a(G_OBJECT_TYPE(p), CppClass::gtype()))
        return (GObjTypeChild*) p;
    else
        return 0;
}

#define MOO_GOBJECT_PROXY_NC(CppClass, ParentCppClass, GObjType, GET_TYPE_MACRO)\
private:                                                                        \
    static GObjectCppProxyImpl *createObject(GObject *gobj)                     \
    {                                                                           \
        return new CppClass(gobj);                                              \
    }                                                                           \
                                                                                \
public:                                                                         \
    GObjType *gobj()                                                            \
    {                                                                           \
        return G_TYPE_CHECK_INSTANCE_CAST(                                      \
            GObjectCppProxyImpl::gobj(), CppClass::gtype(), GObjType);          \
    }                                                                           \
                                                                                \
    const GObjType *gobj() const                                                \
    {                                                                           \
        return const_cast<CppClass*>(this)->gobj();                             \
    }                                                                           \
                                                                                \
    static void registerGType()                                                 \
    {                                                                           \
        static bool beenHere = false;                                           \
        if (!beenHere)                                                          \
        {                                                                       \
            beenHere = true;                                                    \
            ParentCppClass::registerGType();                                    \
            GObjectCppProxyImpl::registerGType(gtype(), createObject);          \
        }                                                                       \
    }                                                                           \
                                                                                \
    static GType gtype()                                                        \
    {                                                                           \
        registerGType();                                                        \
        return GET_TYPE_MACRO;                                                  \
    }                                                                           \
                                                                                \
    static SharedPtr<CppClass> get(GObjType *gobj)                              \
    {                                                                           \
        registerGType();                                                        \
        return static_cast<CppClass*>(GObjectCppProxyImpl::get(G_OBJECT(gobj)));\
    }                                                                           \
                                                                                \
    template<typename OtherGObjType>                                            \
    static SharedPtr<CppClass> get(OtherGObjType *gobj)                         \
    {                                                                           \
        return GObjectTypeHelper<OtherGObjType>::TCppClass::get(gobj);          \
    }

#define MOO_GOBJECT_CONSTRUCTOR(CppClass, ParentCppClass)                       \
protected:                                                                      \
    CppClass(GObject *gobj) : ParentCppClass(gobj) {}

#define MOO_GOBJECT_PROXY(CppClass, ParentCppClass, GObjType, G_TYPE_MACRO)     \
    MOO_GOBJECT_PROXY_NC(CppClass, ParentCppClass, GObjType, G_TYPE_MACRO)      \
    MOO_GOBJECT_CONSTRUCTOR(CppClass, ParentCppClass)

#define MOO_DEFINE_GOBJECT_PROXY_CLASS(CppClass, ParentCppClass,                \
                                       GObjType, G_TYPE_MACRO)                  \
class CppClass : public ParentCppClass                                          \
{                                                                               \
    MOO_GOBJECT_PROXY(CppClass, ParentCppClass, GObjType, G_TYPE_MACRO)         \
}

#define MOO_DEFINE_GOBJECT_HELPER_CLASS(CppClass, GObjType)                     \
template<> class GObjectTypeHelper<GObjType>                                    \
{                                                                               \
public:                                                                         \
    typedef GObjType TGObjType;                                                 \
    typedef CppClass TCppClass;                                                 \
}

namespace G {
MOO_DEFINE_GOBJECT_PROXY_CLASS(Object, GObjectCppProxyImpl, GObject, G_TYPE_OBJECT);
}
MOO_DEFINE_GOBJECT_HELPER_CLASS(G::Object, GObject);

} // namespace moo

#endif /* MOO_CPP_GOBJECT_H */
/* -%- strip:true -%- */
