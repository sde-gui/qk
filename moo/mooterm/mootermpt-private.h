/*
 *   mootermpt-private.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOOTERM_COMPILATION
#error "This file may not be included"
#endif

#ifndef MOO_TERM_PT_PRIVATE_H
#define MOO_TERM_PT_PRIVATE_H

#include "mooterm/mootermpt.h"
#include <string.h>

G_BEGIN_DECLS


#define MOO_TYPE_TERM_PT_CYG        (_moo_term_pt_cyg_get_type ())
#define MOO_TYPE_TERM_PT_UNIX       (_moo_term_pt_unix_get_type ())

struct _MooTermPt {
    GObject             parent;

    MooTermIOFunc       io_func;
    gpointer            io_func_data;
    MooTermIOSizeFunc   size_func;
    gpointer            size_func_data;

    gboolean            child_alive;
    gboolean            alive;
    GQueue             *pending_write;  /* list->data is GByteArray* */
    guint               pending_write_id;

    int                 priority;
    int                 width;
    int                 height;
    guint               echo : 1;
};

struct _MooTermPtClass {
    GObjectClass  parent_class;

    /* virtual methods */
    void        (*set_size)             (MooTermPt  *pt,
                                         guint       width,
                                         guint       height);
    void        (*set_echo_input)       (MooTermPt  *pt,
                                         gboolean    echo);
    gboolean    (*fork_command)         (MooTermPt  *pt,
                                         const MooTermCommand *cmd,
                                         GError    **error);
    void        (*write)                (MooTermPt  *pt,
                                         const char *data,
                                         gssize      len);
    void        (*kill_child)           (MooTermPt  *pt);
    char        (*get_erase_char)       (MooTermPt  *pt);
    void        (*send_intr)            (MooTermPt  *pt);
    gboolean    (*set_fd)               (MooTermPt  *pt,
                                         int         master);

    /* signals */
    void        (*child_died)   (MooTermPt  *pt);
};


GType   _moo_term_pt_unix_get_type          (void) G_GNUC_CONST;
GType   _moo_term_pt_cyg_get_type           (void) G_GNUC_CONST;

void    _moo_term_pt_process_data           (MooTermPt  *pt,
                                             const char *data,
                                             int         len);
gsize   _moo_term_pt_get_input_chunk_len    (MooTermPt  *pt,
                                             gsize       max_len);


inline static void pt_discard (GSList **list)
{
    GSList *l;

    for (l = *list; l != NULL; ++l)
        g_byte_array_free (l->data, TRUE);

    g_slist_free (*list);
    *list = NULL;
}

inline static void pt_discard_pending_write (MooTermPt *pt)
{
    GList *l;

    for (l = pt->pending_write->head; l != NULL; l = l->next)
        g_byte_array_free (l->data, TRUE);

    while (!g_queue_is_empty (pt->pending_write))
        g_queue_pop_head (pt->pending_write);
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

#endif /* MOO_TERM_PT_PRIVATE_H */
