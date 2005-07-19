/*
 *   mooterm/mootermpt-private.h
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

#ifndef MOOTERM_MOOTERMPT_PRIVATE_H
#define MOOTERM_MOOTERMPT_PRIVATE_H

#include "mooterm/mootermpt.h"
#include <string.h>

G_BEGIN_DECLS


struct _MooTermPtPrivate {
    struct _MooTerm *term;
    gboolean         child_alive;
    GQueue          *pending_write;  /* list->data is GByteArray* */
    guint            pending_write_id;
};


inline static void pt_discard (GSList **list)
{
    GSList *l;

    for (l = *list; l != NULL; ++l)
        g_byte_array_free (l->data, TRUE);

    g_slist_free (*list);
    *list = NULL;
}

inline static void pt_flush_pending_write (MooTermPt *pt)
{
    GList *l;

    for (l = pt->priv->pending_write->head; l != NULL; l = l->next)
        g_byte_array_free (l->data, TRUE);

    while (!g_queue_is_empty (pt->priv->pending_write))
        g_queue_pop_head (pt->priv->pending_write);
}

inline static void pt_add_data (GSList **list, const char *data, gssize len)
{
    if (data && len && (len > 0 || data[0]))
    {
        GByteArray *ar;

        if (len < 0)
            len = strlen (data);

        ar = g_byte_array_sized_new ((guint) len);
        *list = g_slist_append (*list,
                                 g_byte_array_append (ar, (const guint8 *)data, len));
    }
}


G_END_DECLS

#endif /* MOOTERM_MOOTERMPT_PRIVATE_H */
