#ifndef MOM_SCRIPT_VARIANT_H
#define MOM_SCRIPT_VARIANT_H

#include "mooscript-types.h"

namespace mom {

enum VariantType
{
    VtVoid,
    VtBool,
    VtIndex,
    VtBase1,
    VtInt,
    VtDouble,
    VtString,
    VtArray,
    VtArgs,
    VtDict,
    VtObject,
};

class Variant;

class VariantArray : public moo::Vector<Variant>
{
public:
    VariantArray() : moo::Vector<Variant>() {}
};

class ArgArray : public VariantArray
{
public:
    ArgArray() {}
    ArgArray(const VariantArray &ar) : VariantArray(ar) {}
};

class VariantDict : public moo::Dict<String, Variant>
{
public:
    VariantDict() : moo::Dict<String, Variant>() {}
};

union VariantData
{
    char p[1];
    gint64 dummy1__;
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
    typename impl::VtHelper<_vt_>::ImplType value() const;

    bool to_bool() const NOTHROW;

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

#define MOM_DEFINE_VT_HELPER(_vt_, _ImplType_)              \
template<> class VtHelper<_vt_>                             \
{                                                           \
public:                                                     \
    static const VariantType vt = _vt_;                     \
    typedef _ImplType_ ImplType;                            \
};                                                          \
                                                            \
template<> class VtRHelper<_ImplType_>                      \
{                                                           \
public:                                                     \
    static const VariantType vt = _vt_;                     \
    typedef _ImplType_ ImplType;                            \
};

MOM_DEFINE_VT_HELPER(VtBool, bool)
MOM_DEFINE_VT_HELPER(VtIndex, Index)
MOM_DEFINE_VT_HELPER(VtBase1, Base1Int)
MOM_DEFINE_VT_HELPER(VtInt, gint64)
MOM_DEFINE_VT_HELPER(VtDouble, double)
MOM_DEFINE_VT_HELPER(VtObject, HObject)
MOM_DEFINE_VT_HELPER(VtString, String)
MOM_DEFINE_VT_HELPER(VtArray, VariantArray)
MOM_DEFINE_VT_HELPER(VtArgs, ArgArray)
MOM_DEFINE_VT_HELPER(VtDict, VariantDict)

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
    MOM_VT_CASE(VtIndex, what);     \
    MOM_VT_CASE(VtBase1, what);     \
    MOM_VT_CASE(VtInt, what);       \
    MOM_VT_CASE(VtDouble, what);    \
    MOM_VT_CASE(VtObject, what);    \
    MOM_VT_CASE(VtString, what);    \
    MOM_VT_CASE(VtArray, what);     \
    MOM_VT_CASE(VtArgs, what);      \
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
inline typename impl::VtHelper<_vt_>::ImplType Variant::value() const
{
    typedef typename impl::VtHelper<_vt_>::ImplType ImplType;
    moo_return_val_if_fail(m_vt == _vt_, ImplType());
    return *reinterpret_cast<const ImplType*>(m_data.p);
}

NOTHROW inline bool Variant::to_bool() const
{
    switch (m_vt)
    {
        case VtVoid:
            return false;
        case VtBool:
            return value<VtBool>();
        default:
            moo_warning("can't convert value to bool");
            return false;
    }
}

} // namespace mom

#endif // MOM_SCRIPT_VARIANT_H
