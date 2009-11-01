#ifndef MOO_LIST_H
#define MOO_LIST_H

#include <glib.h>

#define _MOO_DEFINE_LIST(GListType, glisttype,                          \
                         ListType, list_type, ElmType)                  \
                                                                        \
typedef struct ListType ListType;                                       \
struct ListType {                                                       \
    ElmType *data;                                                      \
    ListType *next;                                                     \
};                                                                      \
                                                                        \
inline static ListType *                                                \
list_type##_from_g##glisttype (GListType *list)                         \
{                                                                       \
    return (ListType*) list;                                            \
}                                                                       \
                                                                        \
inline static GListType *                                               \
list_type##_to_g##glisttype (ListType *list)                            \
{                                                                       \
    return (GListType*) list;                                           \
}                                                                       \
                                                                        \
inline static void                                                      \
list_type##_free_links (ListType *list)                                 \
{                                                                       \
    g_##glisttype##_free ((GListType*) list);                           \
}                                                                       \
                                                                        \
inline static ListType *                                                \
list_type##_copy_links (ListType *list)                                 \
{                                                                       \
    return (ListType*) g_##glisttype##_copy ((GListType*) list);        \
}                                                                       \
                                                                        \
inline static guint                                                     \
list_type##_length (ListType *list)                                     \
{                                                                       \
    return g_##glisttype##_length ((GListType*) list);                  \
}                                                                       \
                                                                        \
inline static ListType *                                                \
list_type##_reverse (ListType *list)                                    \
{                                                                       \
    return (ListType*) g_##glisttype##_reverse ((GListType*) list);     \
}                                                                       \
                                                                        \
inline static ListType *                                                \
list_type##_prepend (ListType *list, ElmType *data)                     \
{                                                                       \
    return (ListType*) g_##glisttype##_prepend ((GListType*)list, data);\
}                                                                       \
                                                                        \
inline static ListType *                                                \
list_type##_append (ListType *list, ElmType *data)                      \
{                                                                       \
    return (ListType*) g_##glisttype##_append ((GListType*)list, data); \
}                                                                       \
                                                                        \
inline static ListType *                                                \
list_type##_concat (ListType *list1, ListType *list2)                   \
{                                                                       \
    return (ListType*) g_##glisttype##_concat ((GListType*) list1,      \
                                       (GListType*) list2);             \
}                                                                       \
                                                                        \
inline static ListType *                                                \
list_type##_remove (ListType *list, const ElmType *data)                \
{                                                                       \
    return (ListType*) g_##glisttype##_remove ((GListType*)list, data); \
}                                                                       \
                                                                        \
inline static ListType *                                                \
list_type##_delete_link (ListType *list, ListType *link)                \
{                                                                       \
    return (ListType*)                                                  \
        g_##glisttype##_delete_link ((GListType*)list,                  \
                                     (GListType*)link);                 \
}                                                                       \
                                                                        \
inline static ListType *                                                \
list_type##_find (ListType *list, const ElmType *data)                  \
{                                                                       \
    return (ListType*) g_##glisttype##_find ((GListType*) list, data);  \
}                                                                       \
                                                                        \
inline static ListType *                                                \
list_type##_find_custom (ListType      *list,                           \
                         gconstpointer  data,                           \
                         GCompareFunc   func)                           \
{                                                                       \
    return (ListType*) g_##glisttype##_find_custom ((GListType*) list,  \
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
    g_##glisttype##_foreach ((GListType*) list,                         \
                             (GFunc) func, user_data);                  \
}

#define MOO_DEFINE_LIST_COPY_FUNC(ListType, list_type, elm_copy_func)   \
inline static ListType *                                                \
list_type##_copy (ListType *list)                                       \
{                                                                       \
    ListType *copy = NULL;                                              \
    while (list)                                                        \
    {                                                                   \
        copy = list_type##_prepend (copy, elm_copy_func (list->data));  \
        list = list->next;                                              \
    }                                                                   \
    return list_type##_reverse (copy);                                  \
}

#define MOO_DEFINE_LIST_FREE_FUNC(ListType, list_type, elm_free_func)   \
inline static void                                                      \
list_type##_free (ListType *list)                                       \
{                                                                       \
    list_type##_foreach (list, (ListType##Func) elm_free_func, NULL);   \
    list_type##_free_links (list);                                      \
}

#define MOO_DEFINE_SLIST(ListType, list_type, ElmType)                  \
    _MOO_DEFINE_LIST(GSList, slist, ListType, list_type, ElmType)

#define MOO_DEFINE_DLIST(ListType, list_type, ElmType)                  \
    _MOO_DEFINE_LIST(GList, list, ListType, list_type, ElmType)

#define MOO_DEFINE_SLIST_FULL(ListType, list_type, ElmType,             \
                              elm_copy_func, elm_free_func)             \
    MOO_DEFINE_SLIST(ListType, list_type, ElmType)                      \
    MOO_DEFINE_LIST_COPY_FUNC(ListType, list_type, elm_copy_func)       \
    MOO_DEFINE_LIST_FREE_FUNC(ListType, list_type, elm_free_func)

#define MOO_DEFINE_DLIST_FULL(ListType, list_type, ElmType,             \
                              elm_copy_func, elm_free_func)             \
    MOO_DEFINE_DLIST(ListType, list_type, ElmType)                      \
    MOO_DEFINE_LIST_COPY_FUNC(ListType, list_type, elm_copy_func)       \
    MOO_DEFINE_LIST_FREE_FUNC(ListType, list_type, elm_free_func)

#endif /* MOO_LIST_H */
