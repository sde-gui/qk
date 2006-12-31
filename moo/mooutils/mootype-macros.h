/*
 *   mootype-macros.h
 *
 *   Copyright (C) 2004-2006 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef __MOO_TYPE_MACROS_H__
#define __MOO_TYPE_MACROS_H__

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
    type_id =                                                                               \
        g_type_register_static (TYPE_PARENT, #TypeName, &type_info, flags);
#else
#define _MOO_REGISTER_TYPE(TypeName,type_name,TYPE_PARENT,flags)                            \
    type_id =                                                                               \
        g_type_register_static_simple (TYPE_PARENT,                                         \
                                       g_intern_static_string (#TypeName),                  \
                                       sizeof (TypeName##Class),                            \
                                       (GClassInitFunc) type_name##_class_intern_init,      \
                                       sizeof (TypeName),                                   \
                                       (GInstanceInitFunc) type_name##_init,                \
                                       flags);
#endif


#define MOO_DEFINE_TYPE_STATIC(TypeName,type_name,TYPE_PARENT)                              \
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
    static GType type_id = 0;                                                               \
                                                                                            \
    if (G_UNLIKELY (!type_id))                                                              \
    {                                                                                       \
        _MOO_REGISTER_TYPE(TypeName,type_name,TYPE_PARENT,0)                                \
    }                                                                                       \
                                                                                            \
    return type_id;                                                                         \
} /* closes type_name##_get_type() */


#endif /* __MOO_TYPE_MACROS_H__ */
