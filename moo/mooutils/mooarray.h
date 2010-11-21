#ifndef MOO_ARRAY_H
#define MOO_ARRAY_H

#include <mooutils/mooutils-mem.h>

#define MOO_DEFINE_PTR_ARRAY(ArrayType, array_type, ElmType,            \
                             copy_elm, free_elm)                        \
                                                                        \
typedef struct ArrayType ArrayType;                                     \
struct ArrayType {                                                      \
    MOO_IP_ARRAY_ELMS (ElmType*, elms);                                 \
};                                                                      \
                                                                        \
inline static ArrayType *                                               \
array_type##_new (void)                                                 \
{                                                                       \
    ArrayType *ar = g_slice_new0 (ArrayType);                           \
    MOO_IP_ARRAY_INIT (ar, elms, 0);                                    \
    return ar;                                                          \
}                                                                       \
                                                                        \
inline static void                                                      \
array_type##_free (ArrayType *ar)                                       \
{                                                                       \
    if (ar)                                                             \
    {                                                                   \
        gsize i;                                                        \
        for (i = 0; i < ar->n_elms; ++i)                                \
            free_elm (ar->elms[i]);                                     \
        MOO_IP_ARRAY_DESTROY (ar, elms);                                \
        g_slice_free (ArrayType, ar);                                   \
    }                                                                   \
}                                                                       \
                                                                        \
inline static void                                                      \
array_type##_append (ArrayType *ar, ElmType *elm)                       \
{                                                                       \
    g_return_if_fail (ar != NULL && elm != NULL);                       \
    MOO_IP_ARRAY_GROW (ar, elms, 1);                                    \
    ar->elms[ar->n_elms - 1] = copy_elm (elm);                          \
}                                                                       \
                                                                        \
inline static void                                                      \
array_type##_take (ArrayType *ar, ElmType *elm)                         \
{                                                                       \
    g_return_if_fail (ar != NULL && elm != NULL);                       \
    MOO_IP_ARRAY_GROW (ar, elms, 1);                                    \
    ar->elms[ar->n_elms - 1] = elm;                                     \
}                                                                       \
                                                                        \
inline static ArrayType *                                               \
array_type##_copy (ArrayType *ar)                                       \
{                                                                       \
    ArrayType *copy;                                                    \
                                                                        \
    g_return_val_if_fail (ar != NULL, NULL);                            \
                                                                        \
    copy = array_type##_new ();                                         \
                                                                        \
    if (ar->n_elms)                                                     \
    {                                                                   \
        guint i;                                                        \
        MOO_IP_ARRAY_GROW (copy, elms, ar->n_elms);                     \
        for (i = 0; i < ar->n_elms; ++i)                                \
            copy->elms[i] = copy_elm (ar->elms[i]);                     \
    }                                                                   \
                                                                        \
    return copy;                                                        \
}                                                                       \
                                                                        \
typedef void (*ArrayType##ForeachFunc) (ElmType *elm,                   \
                                        gpointer data);                 \
                                                                        \
inline static void                                                      \
array_type##_foreach (ArrayType *ar,                                    \
                      ArrayType##ForeachFunc func,                      \
                      gpointer data)                                    \
{                                                                       \
    guint i;                                                            \
    g_return_if_fail (ar != NULL && func != NULL);                      \
    for (i = 0; i < ar->n_elms; ++i)                                    \
        func (ar->elms[i], data);                                       \
}


#define MOO_DEFINE_OBJECT_ARRAY(ArrayType, array_type, ElmType)         \
    MOO_DEFINE_PTR_ARRAY (ArrayType, array_type, ElmType,               \
                          g_object_ref, g_object_unref)

#define MOO_DEFINE_PTR_ARRAY_NO_COPY(ArrayType, array_type, ElmType)    \
inline static ElmType *                                                 \
array_type##_dummy_copy_elm__ (ElmType *elm)                            \
{                                                                       \
    return elm;                                                         \
}                                                                       \
                                                                        \
inline static void                                                      \
array_type##_dummy_free_elm__ (G_GNUC_UNUSED ElmType *elm)              \
{                                                                       \
}                                                                       \
                                                                        \
MOO_DEFINE_PTR_ARRAY (ArrayType, array_type, ElmType,                   \
                      array_type##_dummy_copy_elm__,                    \
                      array_type##_dummy_free_elm__)

#endif /* MOO_ARRAY_H */
