#ifndef MOO_LIST_H
#define MOO_LIST_H

#include <glib.h>

#define MOO_DEFINE_SLIST(ListType, list_type, ElmType)                  \
typedef struct ListType ListType;                                       \
struct ListType {                                                       \
    ElmType *data;                                                      \
    ListType *next;                                                     \
};                                                                      \
                                                                        \
inline static ListType *                                                \
list_type##_from_gslist (GSList *list)                                  \
{                                                                       \
    return (ListType*) list;                                            \
}                                                                       \
                                                                        \
inline static GSList *                                                  \
list_type##_to_gslist (ListType *list)                                  \
{                                                                       \
    return (GSList*) list;                                              \
}                                                                       \
                                                                        \
inline static void                                                      \
list_type##_free (ListType *list)                                       \
{                                                                       \
    g_slist_free ((GSList*) list);                                      \
}                                                                       \
                                                                        \
inline static ListType *                                                \
list_type##_copy (ListType *list)                                       \
{                                                                       \
    return (ListType*) g_slist_copy ((GSList*) list);                   \
}                                                                       \
                                                                        \
inline static ListType *                                                \
list_type##_reverse (ListType *list)                                    \
{                                                                       \
    return (ListType*) g_slist_reverse ((GSList*) list);                \
}                                                                       \
                                                                        \
inline static ListType *                                                \
list_type##_prepend (ListType *list, ElmType *data)                     \
{                                                                       \
    return (ListType*) g_slist_prepend ((GSList*)list, data);           \
}                                                                       \
                                                                        \
inline static ListType *                                                \
list_type##_append (ListType *list, ElmType *data)                      \
{                                                                       \
    return (ListType*) g_slist_append ((GSList*)list, data);            \
}                                                                       \
                                                                        \
inline static ListType *                                                \
list_type##_concat (ListType *list1, ListType *list2)                   \
{                                                                       \
    return (ListType*) g_slist_concat ((GSList*) list1,                 \
                                       (GSList*) list2);                \
}                                                                       \
                                                                        \
inline static ListType *                                                \
list_type##_remove (ListType *list, const ElmType *data)                \
{                                                                       \
    return (ListType*) g_slist_remove ((GSList*)list, data);            \
}                                                                       \
                                                                        \
inline static ListType *                                                \
list_type##_find (ListType *list, const ElmType *data)                  \
{                                                                       \
    return (ListType*) g_slist_find ((GSList*) list, data);             \
}                                                                       \
                                                                        \
inline static ListType *                                                \
list_type##_find_custom (ListType      *list,                           \
                         gconstpointer  data,                           \
                         GCompareFunc   func)                           \
{                                                                       \
    return (ListType*) g_slist_find_custom ((GSList*) list,             \
                                            data, func);                \
}                                                                       \
                                                                        \
typedef void (*ListType##Func) (ElmType *data, gpointer user_data);     \
                                                                        \
inline static void                                                      \
list_type##_foreach (ListType      *list,                               \
                     ListType##Func func,                               \
                     gpointer       user_data)                          \
{                                                                       \
    g_slist_foreach ((GSList*) list, (GFunc) func, user_data);          \
}

#endif /* MOO_LIST_H */
