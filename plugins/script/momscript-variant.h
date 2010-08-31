#ifndef MOM_SCRIPT_VARIANT_H
#define MOM_SCRIPT_VARIANT_H

#include <glib.h>
#include <mooutils/moocpp-cont.h>

namespace mom {

using moo::String;

typedef gint8 int8;
typedef guint8 uint8;
typedef gint16 int16;
typedef guint16 uint16;
typedef gint32 int32;
typedef guint32 uint32;
typedef gint64 int64;
typedef guint64 uint64;

enum VariantType
{
    VtVoid,
    VtBool,
    VtInt32,
    VtUInt32,
    VtInt64,
    VtUInt64,
    VtDouble,
    VtString,
    VtArray,
    VtDict,
    VtObject,
    _VtMax = VtObject
};

class Variant;
class Object;

class HObject
{
public:
    HObject(guint id = 0) NOTHROW : m_id(id) {}
    HObject(const Object &obj) NOTHROW;
    guint id() const NOTHROW { return m_id; }
    bool is_null() const NOTHROW { return m_id == 0; }
private:
    guint m_id;
};

class VariantArray : public moo::Vector<Variant>
{
public:
    VariantArray() : moo::Vector<Variant>() {}
};

class VariantDict : public moo::Dict<String, Variant>
{
public:
    VariantDict() : moo::Dict<String, Variant>() {}
};

union VariantData
{
    char p[1];
    int64 dummy1__;
    double dummy2__;
    long double dummy3__;
    void *dummy4__[2];
};

MOO_STATIC_ASSERT(sizeof(String) <= sizeof(VariantData), "must fit into Variant");
MOO_STATIC_ASSERT(sizeof(VariantArray) <= sizeof(VariantData), "must fit into Variant");
MOO_STATIC_ASSERT(sizeof(VariantDict) <= sizeof(VariantDict), "must fit into Variant");
MOO_STATIC_ASSERT(sizeof(HObject) <= sizeof(VariantData), "must fit into Variant");

namespace impl {
template<VariantType vt>
class VtHelper;
template<typename T>
class VtRHelper;
}

class Variant {
public:
    Variant() NOTHROW;
    ~Variant() NOTHROW;
    Variant(const Variant &v);
    Variant &operator=(const Variant &v);

    NOTHROW VariantType vt() const { return m_vt; }
    NOTHROW bool isVoid() const { return m_vt == VtVoid; }
    void reset() NOTHROW;

    template<typename T>
    Variant(const T &val);

    template<typename T>
    void setValue(const T &val);

    template<VariantType _vt_> NOTHROW
    typename impl::VtHelper<_vt_>::PubType value() const;

private:
    VariantData m_data;
    VariantType m_vt;
};

namespace impl {

template<typename Data>
struct DataPtr
{
private:
    moo::util::CowPtr<Data> m_impl;

public:
    DataPtr() {}
    ~DataPtr() {}

    DataPtr(const Data &data)
        : m_impl(data)
    {
    }

    DataPtr &operator=(const Data &data)
    {
        if (&m_impl.getConst()->data != &data)
            m_impl->data = data;
        return *this;
    }

    operator const Data& () const { return m_impl->data; }
};

MOO_STATIC_ASSERT(sizeof(DataPtr<Variant>) <= sizeof(VariantData), "must fit into Variant");

#define MOM_DEFINE_VT_HELPER(_vt_, _ImplType_, _PubType_)   \
template<> class VtHelper<_vt_>                             \
{                                                           \
public:                                                     \
    static const VariantType vt = _vt_;                     \
    typedef _ImplType_ ImplType;                            \
    typedef _PubType_ PubType;                              \
};                                                          \
                                                            \
template<> class VtRHelper<_PubType_>                       \
{                                                           \
public:                                                     \
    static const VariantType vt = _vt_;                     \
    typedef _ImplType_ ImplType;                            \
    typedef _PubType_ PubType;                              \
};

MOM_DEFINE_VT_HELPER(VtBool, bool, bool)
MOM_DEFINE_VT_HELPER(VtInt32, int32, int32)
MOM_DEFINE_VT_HELPER(VtUInt32, uint32, uint32)
MOM_DEFINE_VT_HELPER(VtInt64, int64, int64)
MOM_DEFINE_VT_HELPER(VtUInt64, uint64, uint64)
MOM_DEFINE_VT_HELPER(VtDouble, double, double)
MOM_DEFINE_VT_HELPER(VtObject, HObject, HObject)
MOM_DEFINE_VT_HELPER(VtString, String, String)
MOM_DEFINE_VT_HELPER(VtArray, VariantArray, VariantArray)
MOM_DEFINE_VT_HELPER(VtDict, VariantDict, VariantDict)

#undef MOM_DEFINE_VT_HELPER

template<typename T> NOTHROW
inline void destroyTyped(VariantData &data)
{
    reinterpret_cast<T*>(data.p)->~T();
}

#define MOM_VT_CASE(vt,what)        \
    case vt: what(vt); break

#define MOM_VT_CASES(what)          \
    MOM_VT_CASE(VtBool, what);      \
    MOM_VT_CASE(VtInt32, what);     \
    MOM_VT_CASE(VtUInt32, what);    \
    MOM_VT_CASE(VtInt64, what);     \
    MOM_VT_CASE(VtUInt64, what);    \
    MOM_VT_CASE(VtDouble, what);    \
    MOM_VT_CASE(VtObject, what);    \
    MOM_VT_CASE(VtString, what);    \
    MOM_VT_CASE(VtArray, what);     \
    MOM_VT_CASE(VtDict, what);

NOTHROW inline void destroy(VariantType vt, VariantData &data)
{
    switch (vt)
    {
        case VtVoid: break;

#define MOM_VT_DESTROY_CASE(VT) destroyTyped<VtHelper<VT>::ImplType>(data)
        MOM_VT_CASES(MOM_VT_DESTROY_CASE)
#undef MOM_VT_DESTROY_CASE
    }
}

template<VariantType vt> NOTHROW
inline typename VtHelper<vt>::ImplType &cast(VariantData &data)
{
    typedef typename VtHelper<vt>::ImplType ImplType;
    return *reinterpret_cast<ImplType*>(data.p);
}

template<VariantType vt> NOTHROW
inline const typename VtHelper<vt>::ImplType &cast(const VariantData &data)
{
    typedef typename VtHelper<vt>::ImplType ImplType;
    return *reinterpret_cast<const ImplType*>(data.p);
}

template<typename T> NOTHROW
inline T &cast(VariantData &data)
{
    return *reinterpret_cast<T*>(data.p);
}

template<typename T> NOTHROW
inline const T &cast(const VariantData &data)
{
    return *reinterpret_cast<const T*>(data.p);
}

NOTHROW inline void copy(VariantType vt, const VariantData &src, VariantData &dest)
{
    switch (vt)
    {
        case VtVoid: break;
#define MOM_VT_COPY_CASE(VT) cast<VT>(dest) = cast<VT>(src)
        MOM_VT_CASES(MOM_VT_COPY_CASE)
#undef MOM_VT_COPY_CASE
    }
}

NOTHROW inline void copyRaw(VariantType vt, const VariantData &src, VariantData &dest)
{
    switch (vt)
    {
        case VtVoid: break;
#define MOM_VT_COPY_RAW_CASE(VT) new (&dest) VtHelper<VT>::ImplType(cast<VT>(src))
        MOM_VT_CASES(MOM_VT_COPY_RAW_CASE)
#undef MOM_VT_COPY_RAW_CASE
    }
}

}

NOTHROW inline Variant::Variant()
    : m_vt(VtVoid)
{
}

template<typename T>
inline Variant::Variant(const T &val)
    : m_vt(VtVoid)
{
    setValue(val);
}

NOTHROW inline Variant::~Variant()
{
    impl::destroy(m_vt, m_data);
}

inline Variant::Variant(const Variant &v)
    : m_vt(v.m_vt)
{
    impl::copyRaw(v.m_vt, v.m_data, m_data);
}

NOTHROW inline Variant &Variant::operator=(const Variant &v)
{
    if (m_vt == v.m_vt)
    {
        impl::copy(m_vt, v.m_data, m_data);
    }
    else
    {
        impl::destroy(m_vt, m_data);
        m_vt = v.m_vt;
        impl::copyRaw(m_vt, v.m_data, m_data);
    }
    return *this;
}

NOTHROW inline void Variant::reset()
{
    impl::destroy(m_vt, m_data);
    m_vt = VtVoid;
}

template<typename T>
inline void Variant::setValue(const T &value)
{
    const VariantType vt = impl::VtRHelper<T>::vt;

    if (m_vt == vt)
    {
        *reinterpret_cast<T*>(m_data.p) = value;
    }
    else
    {
        impl::destroy(m_vt, m_data);
        m_vt = vt;
        new (&m_data) T(value);
    }
}

template<VariantType _vt_> NOTHROW
inline typename impl::VtHelper<_vt_>::PubType Variant::value() const
{
    typedef typename impl::VtHelper<_vt_>::ImplType ImplType;
    moo_return_val_if_fail(m_vt == _vt_, ImplType());
    return *reinterpret_cast<const ImplType*>(m_data.p);
}

} // namespace mom

#endif // MOM_SCRIPT_VARIANT_H
