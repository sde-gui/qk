#ifndef MOO_SCRIPT_H
#define MOO_SCRIPT_H

#include <glib.h>

G_BEGIN_DECLS

typedef guint64 MomObjectId;

typedef struct {
    MomObjectId id;
} MomHandle;

typedef struct MomStringImpl MomStringImpl;
typedef struct MomByteArrayImpl MomByteArrayImpl;
typedef struct MomListImpl MomListImpl;

typedef struct {
    MomStringImpl *p;
} MomString;

typedef struct {
    MomByteArrayImpl *p;
} MomByteArray;

typedef struct {
    MomListImpl *p;
} MomList;

typedef enum {
    MOM_VALUE_NONE = 0,
    MOM_VALUE_HANDLE,
    MOM_VALUE_STRING,
    MOM_VALUE_BYTE_ARRAY,
    MOM_VALUE_BOOL,
    MOM_VALUE_INT,
    MOM_VALUE_UINT,
    MOM_VALUE_LONG,
    MOM_VALUE_ULONG,
    MOM_VALUE_INT64,
    MOM_VALUE_UINT64,
    MOM_VALUE_INVALID
} MomValueType;

typedef struct {
    MomValueType type;
    union {
        MomHandle h;
        MomString s;
        MomByteArray ba;
        gboolean b;
        int i;
        guint u;
        long l;
        gulong ul;
        gint64 ll;
        guint64 ull;
    } u;
} MomValue;

#define MOM_IS_VALUE(mv) ((mv) != NULL && ((guint) (mv)->type) < MOM_VALUE_INVALID)
#define MOM_VALUE_IS_NONE(mv) ((mv)->type == MOM_VALUE_NONE)

typedef enum {
    MOM_SUCCESS = 0,
    MOM_FAIL = -1,
    MOM_UNEXPECTED = -2
} MomResult;

#define MOM_FAILED(mr) ((mr) < 0)
#define MOM_SUCCEEDED(mr) (!MOM_FAILED(mr))

void            mom_value_init      (MomValue           *mv);
MomResult       mom_value_init_copy (const MomValue     *mv_src,
                                     MomValue           *mv_dst);
void            mom_value_free      (MomValue           *mv);
MomResult       mom_value_copy      (const MomValue     *mv_src,
                                     MomValue           *mv_dst);

MomResult       mom_string_init     (MomString          *ms,
                                     const char         *src);
MomResult       mom_string_copy     (const MomString    *ms_src,
                                     MomString          *ms_dst);
MomResult       mom_string_set      (MomString          *ms,
                                     const char         *str);
const char     *mom_string_get      (MomString          *ms);
void            mom_string_free     (MomString          *ms);

void            mom_handle_init     (MomHandle          *mh);
gboolean        mom_handle_is_valid (MomHandle           mh);

MomResult       mom_byte_array_init (MomByteArray       *mba,
                                     const guchar       *src,
                                     guint               len);
MomResult       mom_byte_array_copy (const MomByteArray *mba_src,
                                     MomByteArray       *mba_dst);
MomResult       mom_byte_array_set  (MomByteArray       *mba,
                                     const guchar       *src,
                                     guint               len);
const guchar   *mom_byte_array_get  (MomByteArray       *mba,
                                     guint              *len);
void            mom_byte_array_free (MomByteArray       *mba);


G_END_DECLS

#endif /* MOO_SCRIPT_H */
