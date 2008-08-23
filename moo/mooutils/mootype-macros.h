/*
 *   mootype-macros.h
 *
 *   Copyright (C) 2004-2008 by Yevgen Muntyan <muntyan@tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_TYPE_MACROS_H
#define MOO_TYPE_MACROS_H

#include <glib-object.h>


#if !GLIB_CHECK_VERSION(2,12,0)
#define _MOO_REGISTER_TYPE(TypeName,type_name,TYPE_PARENT,flags)                            \
    static const GTypeInfo type_info = {                                                    \
        sizeof (TypeName##Class),                                                           \
        (GBaseInitFunc) NULL,                                                               \
        (GBaseFinalizeFunc) NULL,                                                           \
        (GClassInitFunc) type_name##_class_intern_init,                                     \
        (GClassFinalizeFunc) NULL,                                                          \
        NULL,   /* class_data */                                                            \
        sizeof (TypeName),                                                                  \
        0,      /* n_preallocs */                                                           \
        (GInstanceInitFunc) type_name##_init,                                               \
        NULL    /* value_table */                                                           \
    };                                                                                      \
                                                                                            \
    g_define_type_id =                                                                      \
        g_type_register_static (TYPE_PARENT, #TypeName, &type_info, (GTypeFlags) flags);
#else
#define _MOO_REGISTER_TYPE(TypeName,type_name,TYPE_PARENT,flags)                            \
    g_define_type_id =                                                                      \
        g_type_register_static_simple (TYPE_PARENT,                                         \
                                       g_intern_static_string (#TypeName),                  \
                                       sizeof (TypeName##Class),                            \
                                       (GClassInitFunc) type_name##_class_intern_init,      \
                                       sizeof (TypeName),                                   \
                                       (GInstanceInitFunc) type_name##_init,                \
                                       (GTypeFlags) flags);
#endif


#define MOO_DEFINE_TYPE_STATIC_WITH_CODE(TypeName,type_name,TYPE_PARENT,code)               \
                                                                                            \
static GType    type_name##_get_type (void) G_GNUC_CONST;                                   \
static void     type_name##_init              (TypeName        *self);                      \
static void     type_name##_class_init        (TypeName##Class *klass);                     \
static gpointer type_name##_parent_class = NULL;                                            \
                                                                                            \
static void     type_name##_class_intern_init (gpointer klass)                              \
{                                                                                           \
    type_name##_parent_class = g_type_class_peek_parent (klass);                            \
    type_name##_class_init ((TypeName##Class*) klass);                                      \
}                                                                                           \
                                                                                            \
static GType                                                                                \
type_name##_get_type (void)                                                                 \
{                                                                                           \
    static GType g_define_type_id;                                                          \
                                                                                            \
    if (G_UNLIKELY (!g_define_type_id))                                                     \
    {                                                                                       \
        _MOO_REGISTER_TYPE(TypeName,type_name,TYPE_PARENT,0)                                \
        code                                                                                \
    }                                                                                       \
                                                                                            \
    return g_define_type_id;                                                                \
} /* closes type_name##_get_type() */

#define MOO_DEFINE_TYPE_STATIC(TypeName,type_name,TYPE_PARENT)                              \
    MOO_DEFINE_TYPE_STATIC_WITH_CODE (TypeName, type_name, TYPE_PARENT, {})


#endif /* MOO_TYPE_MACROS_H */
