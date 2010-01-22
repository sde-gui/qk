#include "moocpp-gobject.h"

namespace moo {

struct GObjTypeInfo
{
    GObjectCppProxyImpl*(*createObject)(GObject*);
};

static GQuark moo_gobj_quark()
{
    static GQuark q;
    if (G_UNLIKELY(!q))
        q = g_quark_from_static_string("moo-gobject-cpp-proxy");
    return q;
}

static GObjTypeInfo *peekTypeInfo(GType type)
{
    return static_cast<GObjTypeInfo*>(g_type_get_qdata(type, moo_gobj_quark()));
}

static GObjTypeInfo *getTypeInfo(GType type)
{
    GObjTypeInfo *ti = NULL;

    while (true)
    {
        ti = peekTypeInfo(type);
        if (ti != NULL || type == G_TYPE_OBJECT)
            break;
        type = g_type_parent(type);
    }

    g_return_val_if_fail(ti != NULL, NULL);
    return ti;
}

static void setTypeInfo(GType type, GObjTypeInfo *ti)
{
    g_type_set_qdata(type, moo_gobj_quark(), ti);
}

GObjectCppProxyImpl::GObjectCppProxyImpl(GObject *gobj)
    : m_gobj(0)
{
    g_return_if_fail(gobj != NULL && g_object_get_qdata(m_gobj, moo_gobj_quark()) == NULL);
    m_gobj = gobj;
    g_object_set_qdata(m_gobj, moo_gobj_quark(), this);
    g_object_weak_ref(m_gobj, (GWeakNotify) weakNotify, this);
}

GObjectCppProxyImpl::~GObjectCppProxyImpl()
{
    if (m_gobj)
    {
        g_critical("%s: oops", G_STRLOC);
        g_object_set_qdata(m_gobj, moo_gobj_quark(), NULL);
        g_object_weak_unref(m_gobj, (GWeakNotify) weakNotify, this);
        m_gobj = NULL;
    }
}

void GObjectCppProxyImpl::weakNotify(void *data, GObject *dead)
{
    GObjectCppProxyImpl *self = static_cast<GObjectCppProxyImpl*>(data);
    g_return_if_fail(self->m_gobj == dead);
    self->m_gobj = NULL;
    self->unref();
}

GObjectCppProxyImpl *GObjectCppProxyImpl::get(GObject *gobj)
{
    GObjectCppProxyImpl *proxy;

    g_return_val_if_fail(gobj != NULL, NULL);

    proxy = static_cast<GObjectCppProxyImpl*>(g_object_get_qdata(gobj, moo_gobj_quark()));

    if (!proxy)
    {
        GObjTypeInfo *ti = getTypeInfo(G_OBJECT_TYPE(gobj));
        g_return_val_if_fail(ti != NULL, NULL);
        proxy = ti->createObject(gobj);
    }

    return proxy;
}

void GObjectCppProxyImpl::registerGType(GType type, GObjectCppProxyImpl*(*createFunc)(GObject *gobj))
{
    g_return_if_fail(g_type_is_a(type, G_TYPE_OBJECT));
    g_return_if_fail(createFunc != NULL);
    g_return_if_fail(peekTypeInfo(type) == NULL);

    GObjTypeInfo *ti = g_new0(GObjTypeInfo, 1);

    ti->createObject = createFunc;

    setTypeInfo(type, ti);
}


} // namespace moo
/* -%- strip:true -%- */
