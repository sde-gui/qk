/*
 *   mooclosure.h
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

#ifndef __MOO_CLOSURE_H__
#define __MOO_CLOSURE_H__

#include <glib-object.h>

G_BEGIN_DECLS


#define MOO_TYPE_CLOSURE (moo_closure_get_type ())

typedef struct _MooClosure MooClosure;
typedef struct _MooObjectPtr MooObjectPtr;

typedef void (*MooClosureCall)      (MooClosure *closure);
typedef void (*MooClosureDestroy)   (MooClosure *closure);

struct _MooClosure
{
    MooClosureCall call;
    MooClosureDestroy destroy;
    guint ref_count : 16;
    guint valid : 1;
    guint floating : 1;
    guint in_call : 1;
};

struct _MooObjectPtr
{
    GObject *target;
    GWeakNotify notify;
    gpointer notify_data;
};


GType       moo_closure_get_type            (void) G_GNUC_CONST;

MooClosure *moo_closure_alloc               (gsize size,
                                             MooClosureCall call,
                                             MooClosureDestroy destroy);
#define moo_closure_new(Type__,call__,destroy__) \
    ((Type__*)moo_closure_alloc (sizeof(Type__), call__, destroy__))

MooClosure *moo_closure_ref                 (MooClosure *closure);
void        moo_closure_unref               (MooClosure *closure);
void        moo_closure_sink                (MooClosure *closure);

void        moo_closure_invoke              (MooClosure *closure);
void        moo_closure_invalidate          (MooClosure *closure);


#define MOO_OBJECT_PTR_GET(ptr_) ((ptr_) && (ptr_)->target ? (ptr_)->target : NULL)

MooObjectPtr *moo_object_ptr_new            (GObject    *object,
                                             GWeakNotify notify,
                                             gpointer    data);
void        moo_object_ptr_die              (MooObjectPtr *ptr);
void        moo_object_ptr_free             (MooObjectPtr *ptr);


MooClosure *moo_closure_new_simple          (gpointer    object,
                                             const char *signal,
                                             GCallback   callback,
                                             GCallback   proxy_func);


G_END_DECLS

#endif /* __MOO_CLOSURE_H__ */
/* kate: strip on; indent-width 4; */
