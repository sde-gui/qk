/*
 *   mooutils/mooclosure.h
 *
 *   Copyright (C) 2004-2005 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOOUI_MOOCLOSURE_H
#define MOOUI_MOOCLOSURE_H

#include <gtk/gtkobject.h>

G_BEGIN_DECLS


#define MOO_TYPE_CLOSURE              (moo_closure_get_type ())
#define MOO_CLOSURE(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_CLOSURE, MooClosure))
#define MOO_CLOSURE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_CLOSURE, MooClosureClass))
#define MOO_IS_CLOSURE(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_CLOSURE))
#define MOO_IS_CLOSURE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_CLOSURE))
#define MOO_CLOSURE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_CLOSURE, MooClosureClass))


typedef struct _MooClosure        MooClosure;
typedef struct _MooClosurePrivate MooClosurePrivate;
typedef struct _MooClosureClass   MooClosureClass;

struct _MooClosure
{
    GtkObject    parent_instance;
    void        (*callback)         (gpointer data);
    gpointer    (*proxy_func)       (gpointer data);
    char        *signal;
    gpointer     data;
    guint        object         : 1;
    guint        valid          : 1;
    guint        constructed    : 1;
};

struct _MooClosureClass
{
    GtkObjectClass parent_class;

    void (* invoke) (MooClosure *closure);
};


GType        moo_closure_get_type           (void) G_GNUC_CONST;

MooClosure  *moo_closure_new                (GCallback       callback_func,
                                             gpointer        data);
MooClosure  *moo_closure_new_object         (GCallback       callback_func,
                                             gpointer        object);
MooClosure  *moo_closure_new_signal         (const char     *signal,
                                             gpointer        object);
MooClosure  *moo_closure_new_proxy          (GCallback       callback_func,
                                             GCallback       proxy_func,
                                             gpointer        object);
MooClosure  *moo_closure_new_proxy_signal   (const char     *signal,
                                             GCallback       proxy_func,
                                             gpointer        object);

void         moo_closure_invoke             (MooClosure     *closure);
void         moo_closure_invalidate         (MooClosure     *closure);


G_END_DECLS

#endif /* MOOUI_MOOCLOSURE_H */

