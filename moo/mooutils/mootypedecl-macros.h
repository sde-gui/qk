/*
 *   mootypedecl-macros.h
 *
 *   Copyright (C) 2004-2010 by Yevgen Muntyan <emuntyan@users.sourceforge.net>
 *
 *   This file is part of medit.  medit is free software; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU Lesser General Public License as published by the
 *   Free Software Foundation; either version 2.1 of the License,
 *   or (at your option) any later version.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with medit.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MOO_TYPE_DECL_MACROS_H
#define MOO_TYPE_DECL_MACROS_H

#include <glib-object.h>

#define _MOO_DECLARE_GOBJECT_CLASS(Parent,                              \
                                   PREFIX,Prefix,prefix,                \
                                   TYPE,Type,type)                      \
                                                                        \
typedef struct Prefix##Type Prefix##Type;                               \
typedef struct Prefix##Type##Class Prefix##Type##Class;                 \
typedef struct Prefix##Type##Private Prefix##Type##Private;             \
typedef Parent Prefix##Type##_Base;                                     \
typedef Parent##Class Prefix##Type##_BaseClass;                         \
                                                                        \
struct Prefix##Type                                                     \
{                                                                       \
    Parent base;                                                        \
    Prefix##Type##Private *priv;                                        \
};                                                                      \
                                                                        \
GType prefix##_##type##_get_type (void) G_GNUC_CONST;                   \
                                                                        \
inline static Prefix##Type*                                             \
PREFIX##_##TYPE(void *object)                                           \
{                                                                       \
    return G_TYPE_CHECK_INSTANCE_CAST (object,                          \
                                       prefix##_##type##_get_type (),   \
                                       Prefix##Type);                   \
}                                                                       \
                                                                        \
inline static Prefix##Type##Class*                                      \
PREFIX##_##TYPE##_CLASS(void *klass)                                    \
{                                                                       \
    return G_TYPE_CHECK_CLASS_CAST (klass,                              \
                                    prefix##_##type##_get_type (),      \
                                    Prefix##Type##Class);               \
}                                                                       \
                                                                        \
inline static gboolean                                                  \
PREFIX##_IS_##TYPE(void *object)                                        \
{                                                                       \
    return G_TYPE_CHECK_INSTANCE_TYPE (object,                          \
                                       prefix##_##type##_get_type ());  \
}                                                                       \
                                                                        \
inline static gboolean                                                  \
PREFIX##_IS_##TYPE##_CLASS(void *klass)                                 \
{                                                                       \
    return G_TYPE_CHECK_CLASS_TYPE (klass,                              \
                                    prefix##_##type##_get_type ());     \
}                                                                       \
                                                                        \
inline static Prefix##Type##Class *                                     \
PREFIX##_##TYPE##_GET_CLASS(void *object)                               \
{                                                                       \
    return G_TYPE_INSTANCE_GET_CLASS (object,                           \
                                      prefix##_##type##_get_type (),    \
                                      Prefix##Type##Class);             \
}

#define MOO_DECLARE_GOBJECT_CLASS(TYPE,Type,type,Parent)                \
    _MOO_DECLARE_GOBJECT_CLASS(Parent, MOO, Moo, moo, TYPE, Type, type)

#endif /* MOO_TYPE_DECL_MACROS_H */
/* -%- strip:true -%- */
