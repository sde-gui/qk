#ifndef MOM_OBJECT_H
#define MOM_OBJECT_H

#include "mooscript.h"

#ifdef __cplusplus

#include <vector>
#include <string>

namespace mom
{

typedef MomResult Result;
typedef std::string String;
typedef std::vector<guchar> ByteArray;

class _ObjectImpl
{
    MomHandle m_handle;

protected:
    _ObjectImpl();

public:
    bool implIsValid() const;
    MomHandle implHandle() const;
    void implSetHandle(MomHandle handle);
};

template<class T>
class List
{
    std::vector<T> m_data;

public:
    Result count(guint &cnt);
    Result item(guint idx, T &itm);
};

namespace impl
{

struct MomMethod;
struct MomClass;

struct TypeInfo
{
    MomValueType val_type;
    MomClass *pclass;
};

enum PropertyFlags
{
    PROP_READ      = 1 << 0,
    PROP_WRITE     = 1 << 1,
    PROP_READWRITE = PROP_READ | PROP_WRITE
};

struct PropertyInfo
{
    MomClass *pclass;
    const char *setter;
    const char *getter;
    TypeInfo type;
    PropertyFlags flags;
};

enum ArgFlags
{
    ARG_FLAG_RESERVED = 1 << 0
};

struct ArgInfo
{
    TypeInfo type;
    ArgFlags flags;
};

struct MomMethod
{
    MomClass *pclass;
    TypeInfo ret_type;
    ArgInfo *args;
    guint n_args;
    guint id;
};

struct MomClass
{
    MomClass *parent;
    GHashTable *methods;
    GHashTable *props;

    MomMethod &add_method(const char *name);
    PropertyInfo &add_property(const char *name, const TypeInfo &type, PropertyFlags flags);

    bool is_subclass_of(MomClass *pclass);
    MomMethod *lookup_method(const char *name);

    Result call_method(MomObject *obj, MomMethod *meth, const MomValue *args, guint n_args, MomValue *ret);
};

class MomObject
{
private:
    MomClass *m_pclass;
    MomObjectId m_id;
    static MomObjectId s_last_id;

public:
    union {
    } u;

private:
    MomObject();
    MomObject(const MomObject&);
    ~MomObject();
    MomObject &operator=(const MomObject&);

public:
    static MomObject &by_id(MomObjectId id);
    static MomObject &new_object(MomClass *pclass);
    static void destroy_object(MomObject &obj);

    MomObjectId get_id() const;
    bool is_instance_of(MomClass *pclass);
    MomClass *get_class() const;

    Result call_method(const char *method, const MomValue *args, guint n_args, MomValue *ret);
    Result set_property(const char *property, const MomValue *val);
    Result get_property(const char *property, MomValue *val);
};

class TypeStore
{
    GHashTable *classes;

    TypeStore(const TypeStore&);
    TypeStore &operator = (const TypeStore&);

public:
    TypeStore();
    ~TypeStore();

    MomClass &register_class (const char *name, const char *parent);
    MomClass *lookup_class (const char *name);
};

} // namespace impl

} // namespace mom

#define MOM_DECLARE_OBJECT_CLASS(Class)                         \
public:                                                         \
    static const char *_class_name() { return #Class; }

#endif /* __cplusplus */

#endif /* MOM_OBJECT_H */
